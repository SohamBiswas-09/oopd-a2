#pragma once

// AcquisitionManager: turns purchase requests into orders, enforcing the
// Budget's quotas, updating Catalog holdings and keeping an order history.
//
// Q9:
// Each department can have its own Budget object.
// A purchase request can specify the department that pays for it.
//
// Q11:
// processBatch() can optionally operate in all-or-nothing mode.
// If allOrNothing is true, the complete batch is committed only when
// every request can be approved.

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
    AcquisitionManager(Catalog& catalog, Budget& budget);

    Money quote(
        const std::string& id,
        int quantity
    ) const;

    void setPrintTaxRate(
        double percent
    );

    void setElectronicTaxRate(
        double percent
    );

    double printTaxRate() const {
        return printTaxRate_;
    }

    double electronicTaxRate() const {
        return electronicTaxRate_;
    }

    // ========================================================
    // Q9: Department budgets
    // ========================================================

    void addDepartment(
        const std::string& department,
        Money budget
    );

    Budget& departmentBudget(
        const std::string& department
    );

    const Budget& departmentBudget(
        const std::string& department
    ) const;

    bool hasDepartment(
        const std::string& department
    ) const;

    // ========================================================
    // Purchase checking
    // ========================================================

    bool canPurchase(
        const std::string& id,
        int quantity,
        std::string* reason = nullptr
    ) const;

    bool canPurchase(
        const std::string& department,
        const std::string& id,
        int quantity,
        std::string* reason = nullptr
    ) const;

    // ========================================================
    // Purchasing
    // ========================================================

    const PurchaseRecord& purchase(
        const std::string& id,
        int quantity
    );

    const PurchaseRecord& purchase(
        const std::string& department,
        const std::string& id,
        int quantity
    );

    // ========================================================
    // Q8: Cancellation
    // ========================================================

    const PurchaseRecord& cancel(
        int orderNo
    );

    // ========================================================
    // Q11: Batch processing
    //
    // allOrNothing = false:
    //     Existing behavior. Each request is processed independently.
    //
    // allOrNothing = true:
    //     The complete batch is committed only if every request
    //     can be approved. If any request would fail, nothing
    //     is purchased.
    // ========================================================

    std::vector<PurchaseRecord> processBatch(
        const std::vector<PurchaseRequest>& reqs,
        bool allOrNothing = false
    );

    const std::vector<PurchaseRecord>& history() const {
        return history_;
    }

    Money totalSpent() const;

    void printReport(
        std::ostream& os
    ) const;

private:
    Money taxFor(
        const Resource& r,
        Money preTaxCost
    ) const;

    Budget& budgetFor(
        const std::string& department
    );

    const Budget& budgetFor(
        const std::string& department
    ) const;

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

    Catalog& catalog_;

    // Original/default budget.
    Budget& budget_;

    // Q9: budgets for named departments.
    std::map<std::string, Budget> departmentBudgets_;

    std::vector<PurchaseRecord> history_;

    int nextOrderNo_ = 1;

    double printTaxRate_ = 0.0;

    double electronicTaxRate_ = 0.0;
};

}
