#pragma once
// AcquisitionManager: turns purchase requests into orders, enforcing the
// Budget's quotas, updating Catalog holdings and keeping an order history.
//
// Q9:
// Each department can have its own Budget object.
// A purchase request can specify the department that pays for it.

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "bookmgmt/Budget.h"
#include "bookmgmt/Catalog.h"

namespace bookmgmt {

struct PurchaseRequest {
    std::string resourceId;
    int quantity;  // copies for print, seats for electronic

    // Q9:
    // Empty means use the original/default budget.
    std::string department;
};

struct PurchaseRecord {
    int orderNo;
    std::string resourceId;
    std::string title;
    ResourceCategory category;
    int quantity;

    // Q6: complete tax breakdown.
    Money preTaxCost;
    Money tax;
    Money postTaxCost;

    // Kept for backward compatibility with the original API.
    // This is the final amount charged, i.e. the post-tax cost.
    Money cost;

    bool approved;
    std::string reason;

    // Q8:
    // True when this history entry represents a cancellation.
    bool cancelled = false;

    // Q9:
    // Department charged for this order.
    // Empty means the original/default budget was used.
    std::string department;
};

class AcquisitionManager {
public:
    AcquisitionManager(
        Catalog& catalog,
        Budget& budget
    );

    // --------------------------------------------------
    // Basic purchase quoting
    // --------------------------------------------------

    // Price of a request before tax.
    // Throws NotFoundError.
    Money quote(
        const std::string& id,
        int quantity
    ) const;

    // --------------------------------------------------
    // Q6: Tax configuration
    // --------------------------------------------------

    // Rates are percentages.
    // Example: 5.0 means 5%.
    void setPrintTaxRate(double percent);

    void setElectronicTaxRate(double percent);

    double printTaxRate() const {
        return printTaxRate_;
    }

    double electronicTaxRate() const {
        return electronicTaxRate_;
    }

    // --------------------------------------------------
    // Q9: Department budgets
    // --------------------------------------------------

    // Creates a department with its own budget.
    //
    // Throws std::invalid_argument if the department name
    // is empty or already exists.
    void addDepartment(
        const std::string& department,
        Money budget
    );

    // Returns the budget belonging to a department.
    //
    // Throws std::invalid_argument if the department does
    // not exist.
    Budget& departmentBudget(
        const std::string& department
    );

    const Budget& departmentBudget(
        const std::string& department
    ) const;

    // Returns true if a department has been registered.
    bool hasDepartment(
        const std::string& department
    ) const;

    // --------------------------------------------------
    // Purchase validation
    // --------------------------------------------------

    // Original Q1-Q8 API.
    //
    // Uses the original/default Budget supplied to the
    // AcquisitionManager constructor.
    bool canPurchase(
        const std::string& id,
        int quantity,
        std::string* reason = nullptr
    ) const;

    // Q9:
    // Checks whether a department can afford the purchase
    // and satisfy its own quotas.
    bool canPurchase(
        const std::string& department,
        const std::string& id,
        int quantity,
        std::string* reason = nullptr
    ) const;

    // --------------------------------------------------
    // Purchase
    // --------------------------------------------------

    // Original Q1-Q8 API.
    //
    // Uses the original/default Budget.
    const PurchaseRecord& purchase(
        const std::string& id,
        int quantity
    );

    // Q9:
    // Charges the purchase to the specified department.
    const PurchaseRecord& purchase(
        const std::string& department,
        const std::string& id,
        int quantity
    );

    // --------------------------------------------------
    // Q8: Cancellation
    // --------------------------------------------------

    // Cancels an approved purchase order.
    //
    // The original order remains in history and a separate
    // cancellation record is added.
    //
    // Throws std::invalid_argument if the order does not exist,
    // was not approved, or was already cancelled.
    const PurchaseRecord& cancel(int orderNo);

    // --------------------------------------------------
    // Q8/Q9: Batch processing
    // --------------------------------------------------

    // Original batch API.
    //
    // Each request is approved or rejected independently.
    std::vector<PurchaseRecord> processBatch(
        const std::vector<PurchaseRequest>& reqs
    );

    const std::vector<PurchaseRecord>& history() const {
        return history_;
    }

    Money totalSpent() const;

    void printReport(std::ostream& os) const;

private:
    // --------------------------------------------------
    // Tax calculation
    // --------------------------------------------------

    Money taxFor(
        const Resource& r,
        Money preTaxCost
    ) const;

    // --------------------------------------------------
    // Budget selection
    // --------------------------------------------------

    // Returns the default/original budget when department
    // is empty, otherwise returns the requested department.
    Budget& budgetFor(
        const std::string& department
    );

    const Budget& budgetFor(
        const std::string& department
    ) const;

    // --------------------------------------------------
    // History creation
    // --------------------------------------------------

    PurchaseRecord& record(
        const Resource* r,
        const std::string& id,
        int qty,
        Money preTaxCost,
        Money tax,
        Money postTaxCost,
        bool approved,
        std::string reason,
        bool cancelled = false,
        const std::string& department = {}
    );

    // --------------------------------------------------
    // Data members
    // --------------------------------------------------

    Catalog& catalog_;

    // Original/default budget.
    //
    // Kept so that all Q1-Q8 code continues to work.
    Budget& budget_;

    // Q9:
    // Each department has an independent Budget object.
    std::map<std::string, Budget> departmentBudgets_;

    std::vector<PurchaseRecord> history_;

    int nextOrderNo_ = 1;

    // Q6:
    // Configurable tax rates, expressed as percentages.
    double printTaxRate_ = 0.0;
    double electronicTaxRate_ = 0.0;
};

}  // namespace bookmgmt
