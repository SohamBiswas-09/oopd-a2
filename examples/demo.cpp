// Demo: builds a small catalog, sets a budget with per-category quotas,
// and runs purchase requests through the acquisition manager.
//
// Q8 additionally demonstrates cancellation of an approved order.
// Q9 additionally demonstrates department-specific budgets.

#include <iostream>

#include "bookmgmt/bookmgmt.h"

using namespace bookmgmt;

int main() {
    Catalog catalog;

    catalog.emplace<Book>(
        "B001",
        "Clean Code",
        std::vector<std::string>{"Robert C. Martin"},
        "978-0132350884",
        "Prentice Hall",
        2008,
        Money::of(450)
    );

    catalog.emplace<Book>(
        "B002",
        "The C++ Programming Language",
        std::vector<std::string>{"Bjarne Stroustrup"},
        "978-0321563842",
        "Addison-Wesley",
        2013,
        Money::of(1200),
        4,
        Binding::Hardcover
    );

    catalog.emplace<ElectronicResource>(
        "R001",
        "IEEE Xplore Digital Library",
        "IEEE",
        2026,
        Money::of(150),
        "https://ieeexplore.example",
        LicenseModel::AnnualSubscription,
        Money::of(2000)
    );

    catalog.emplace<ElectronicResource>(
        "R002",
        "MATLAB Campus Licence",
        "MathWorks",
        2026,
        Money::of(400),
        "https://licensing.example/matlab",
        LicenseModel::Perpetual
    );

    catalog.emplace<Journal>(
        "J001",
        "ACM Computing Surveys",
        "1234-5678",
        12,
        "ACM",
        2026,
        Money::of(50),
        2
    );

    // Q2: Add an EBook to the catalog.
    catalog.emplace<EBook>(
        "E001",
        "Design Patterns",
        std::vector<std::string>{"Erich Gamma", "Richard Helm"},
        "978-0201633610",
        "Addison-Wesley",
        1994,
        Money::of(15),
        "https://ebooks.example/design-patterns",
        LicenseModel::AnnualSubscription,
        Money::of(100),
        "EPUB",
        true
    );

    std::cout << "=== Catalog ===\n";

    for (const Resource* r : catalog.all()) {
        std::cout << r->summary() << "\n";
    }

    std::cout << "\n=== Details of R001 ===\n"
              << catalog.get("R001");

    std::cout << "\n=== Details of E001 ===\n"
              << catalog.get("E001");

    Budget budget(Money::of(20000));

    budget.setQuota(
        ResourceCategory::Book,
        {20, Money::of(10000)}
    );

    budget.setQuota(
        ResourceCategory::ElectronicResource,
        {100, Money::of(12000)}
    );

    // Q2: EBook has its own resource category.
    budget.setQuota(
        ResourceCategory::EBook,
        {20, Money::of(5000)}
    );

    AcquisitionManager acq(catalog, budget);

    std::cout << "\n=== Quotes ===\n";

    // Q4: Hardcover books cost 20% more than their listed unit price.
    std::cout << "B002 listed price   = "
              << catalog.get("B002").unitPrice()
              << "\n";

    std::cout << "B002 hardcover cost = "
              << catalog.get("B002").costFor(1)
              << "\n";

    std::cout << "5 copies of B002  = "
              << acq.quote("B002", 5)
              << "\n";

    // Q5: Print items get 10% off for 10 or more copies.
    std::cout << "\n=== Q5 Bulk Discounts ===\n";

    std::cout << "9 copies of B001  = "
              << acq.quote("B001", 9)
              << "  (no bulk discount)\n";

    std::cout << "10 copies of B001 = "
              << acq.quote("B001", 10)
              << "  (10% bulk discount)\n";

    // Q4 + Q5 together:
    // B002 is hardcover, so its listed price first increases by 20%.
    // At 10 copies, the 10% bulk discount is then applied.
    std::cout << "10 copies of B002 = "
              << acq.quote("B002", 10)
              << "  (hardcover + bulk discount)\n";

    std::cout << "\n=== Electronic Seat Pricing ===\n";

    std::cout << "50 seats of R001  = "
              << acq.quote("R001", 50)
              << "  (first 50 at full price)\n";

    std::cout << "60 seats of R001  = "
              << acq.quote("R001", 60)
              << "  (seats beyond 50 at half price)\n";

    std::cout << "5 seats of E001   = "
              << acq.quote("E001", 5)
              << "  (EBook)\n";

    acq.processBatch({
        {"B001", 4},   // 1800  ok
        {"B002", 5},   // 7200  ok
        {"B001", 1},   // 450   ok
        {"R001", 20},  // 5000  ok
        {"R002", 25},  // rejected: electronic spend quota exceeded
        {"R002", 15},  // rejected: overall budget exceeded
        {"R002", 5},   // 2000  ok
        {"E001", 5},   // EBook purchase
        {"X999", 1},   // rejected: unknown id
    });

    std::cout << "\n=== Acquisition report ===\n";
    acq.printReport(std::cout);

    std::cout << "\n=== Budget ===\n";
    budget.print(std::cout);

    std::cout << "\n=== Holdings ===\n";

    for (const Resource* r : catalog.all()) {
        std::cout << "  "
                  << r->id()
                  << ": "
                  << catalog.holdings(r->id())
                  << (r->isDigital() ? " seats" : " copies")
                  << "\n";
    }

    // Direct purchase: errors are reported with exceptions.
    std::cout << "\n=== Direct purchase that breaks a quota ===\n";

    try {
        const PurchaseRecord& rejectedOrder =
            acq.purchase("B002", 1);

        if (!rejectedOrder.approved) {
            std::cout << "Purchase rejected: "
                      << rejectedOrder.reason
                      << "\n";
        }
    } catch (const std::exception& e) {
        std::cout << "Purchase error: "
                  << e.what()
                  << "\n";
    }

    // --------------------------------------------------
    // Q8: Cancellation
    // --------------------------------------------------
    //
    // An approved order is cancelled.
    //
    // The cancellation:
    //   1. refunds the budget,
    //   2. refunds quota usage,
    //   3. reduces catalogue holdings,
    //   4. keeps the original order in history,
    //   5. adds a separate cancellation record.
    // --------------------------------------------------

    std::cout << "\n=== Q8 Cancellation ===\n";

    // One copy costs 450.00.
    //
    // Current Book spending is 9450.00, so:
    //
    //     9450 + 450 = 9900
    //
    // This is within the 10000.00 Book quota.
    const PurchaseRecord& cancellationTestOrder =
        acq.purchase("B001", 1);

    std::cout << "Created order #"
              << cancellationTestOrder.orderNo
              << " for "
              << cancellationTestOrder.quantity
              << " copy of "
              << cancellationTestOrder.title
              << "\n";

    std::cout << "Order approved: "
              << (cancellationTestOrder.approved
                      ? "yes"
                      : "no")
              << "\n";

    std::cout << "Holdings before cancellation: "
              << catalog.holdings("B001")
              << "\n";

    std::cout << "Total spent before cancellation: "
              << acq.totalSpent()
              << "\n";

    if (cancellationTestOrder.approved) {

        const int cancelledOrderNo =
            cancellationTestOrder.orderNo;

        const PurchaseRecord& cancellation =
            acq.cancel(cancelledOrderNo);

        std::cout << "Cancellation record: "
                  << cancellation.reason
                  << "\n";

        std::cout << "Holdings after cancellation: "
                  << catalog.holdings("B001")
                  << "\n";

        std::cout << "Total spent after cancellation: "
                  << acq.totalSpent()
                  << "\n";
    }

    // --------------------------------------------------
    // Q9: Department Budgets
    // --------------------------------------------------
    //
    // Each department has its own independent Budget.
    //
    // Here we create:
    //
    //   Computer Science:
    //       Budget = 5000.00
    //       Book quota = 10 units / 3000.00
    //
    //   Physics:
    //       Budget = 3000.00
    //       Book quota = 5 units / 1500.00
    //
    // Purchases made by one department do not consume
    // the budget of another department.
    // --------------------------------------------------

    std::cout << "\n=== Q9 Department Budgets ===\n";

    acq.addDepartment(
        "Computer Science",
        Money::of(5000)
    );

    acq.addDepartment(
        "Physics",
        Money::of(3000)
    );

    acq.departmentBudget("Computer Science").setQuota(
        ResourceCategory::Book,
        {10, Money::of(3000), 2}
    );

    acq.departmentBudget("Physics").setQuota(
        ResourceCategory::Book,
        {5, Money::of(1500), 1}
    );

    std::cout << "Computer Science budget: "
              << acq.departmentBudget("Computer Science").total()
              << "\n";

    std::cout << "Physics budget: "
              << acq.departmentBudget("Physics").total()
              << "\n";

    // Purchase B001 through Computer Science.
    const PurchaseRecord& csOrder =
        acq.purchase(
            "Computer Science",
            "B001",
            2
        );

    std::cout << "\nComputer Science order #"
              << csOrder.orderNo
              << ": "
              << (csOrder.approved ? "approved" : "rejected")
              << "\n";

    std::cout << "Computer Science spent: "
              << acq.departmentBudget("Computer Science").spent()
              << "\n";

    std::cout << "Physics spent: "
              << acq.departmentBudget("Physics").spent()
              << "\n";

    // Purchase B002 through Physics.
    const PurchaseRecord& physicsOrder =
        acq.purchase(
            "Physics",
            "B002",
            1
        );

    std::cout << "\nPhysics order #"
              << physicsOrder.orderNo
              << ": "
              << (physicsOrder.approved
                      ? "approved"
                      : "rejected")
              << "\n";

    std::cout << "Physics spent: "
              << acq.departmentBudget("Physics").spent()
              << "\n";

    std::cout << "Computer Science spent: "
              << acq.departmentBudget("Computer Science").spent()
              << "\n";

    // Q9: A department-specific batch.
    std::cout << "\nDepartment batch:\n";

    const auto departmentBatch =
        acq.processBatch({
            {"B001", 1, "Computer Science"},
            {"E001", 2, "Computer Science"},
            {"B001", 1, "Physics"}
        });

    for (const PurchaseRecord& order : departmentBatch) {
        std::cout << "  Order #"
                  << order.orderNo
                  << " | Department: "
                  << order.department
                  << " | "
                  << order.title
                  << " | "
                  << (order.approved
                          ? "approved"
                          : "rejected")
                  << "\n";
    }

    std::cout << "\n=== Department Budgets After Purchases ===\n";

    std::cout << "Computer Science spent: "
              << acq.departmentBudget("Computer Science").spent()
              << "\n";

    std::cout << "Computer Science remaining: "
              << acq.departmentBudget("Computer Science").remaining()
              << "\n";

    std::cout << "Physics spent: "
              << acq.departmentBudget("Physics").spent()
              << "\n";

    std::cout << "Physics remaining: "
              << acq.departmentBudget("Physics").remaining()
              << "\n";

    // Q9 + Q8:
    // Cancel the Physics order and demonstrate that only
    // the Physics department receives the refund.
    std::cout << "\n=== Q9 Department Cancellation ===\n";

    if (physicsOrder.approved) {
        std::cout << "Physics spent before cancellation: "
                  << acq.departmentBudget("Physics").spent()
                  << "\n";

        const PurchaseRecord& departmentCancellation =
            acq.cancel(physicsOrder.orderNo);

        std::cout << "Cancellation: "
                  << departmentCancellation.reason
                  << "\n";

        std::cout << "Physics spent after cancellation: "
                  << acq.departmentBudget("Physics").spent()
                  << "\n";

        std::cout << "Computer Science spent after Physics cancellation: "
                  << acq.departmentBudget("Computer Science").spent()
                  << "\n";
    }

    std::cout << "\n=== Final Acquisition Report ===\n";
    acq.printReport(std::cout);

    return 0;
}
