// Demo: builds a small catalog, sets a budget with per-category quotas,
// and runs a batch of purchase requests through the acquisition manager.

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
        {10, Money::of(8000)}
    );

    budget.setQuota(
        ResourceCategory::ElectronicResource,
        {40, Money::of(12000)}
    );

    // Q2: EBook has its own resource category.
    budget.setQuota(
        ResourceCategory::EBook,
        {20, Money::of(5000)}
    );

    AcquisitionManager acq(catalog, budget);

    std::cout << "\n=== Quotes ===\n";

    std::cout << "5 copies of B002  = "
              << acq.quote("B002", 5)
              << "\n";

    std::cout << "20 seats of R001  = "
              << acq.quote("R001", 20)
              << "  (incl. platform fee)\n";

    std::cout << "5 seats of E001   = "
              << acq.quote("E001", 5)
              << "  (EBook)\n";

    acq.processBatch({
        {"B001", 4},   // 1800  ok
        {"B002", 5},   // 6000  ok  -> book spend 7800
        {"B001", 1},   // 450   rejected: book spend quota (200 left)
        {"R001", 20},  // 5000  ok
        {"R002", 25},  // 10000 rejected: e-resource unit quota (20 seats left)
        {"R002", 15},  // 6000  ok  -> e-resource spend 11000
        {"R002", 5},   // 2000  rejected: e-resource spend quota (1000 left)
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
        acq.purchase("B002", 1);
    } catch (const QuotaExceededError& e) {
        std::cout << "QuotaExceededError: "
                  << e.what()
                  << "\n";
    }

    return 0;
}
