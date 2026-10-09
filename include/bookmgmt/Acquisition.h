#pragma once

// AcquisitionManager: processes purchase requests, enforces budgets,
// updates catalogue holdings and maintains order history.
//
// Q9: Department-specific budgets.
// Q11: Optional all-or-nothing batch processing.
// Q12: Multiple vendors and cheapest-price selection.

#include <iosfwd>
#include <map>
#include <string>
#include <vector>

#include "bookmgmt/Budget.h"
#include "bookmgmt/Catalog.h"

namespace bookmgmt {

struct PurchaseRequest {
    std::string resourceId;
    int quantity;
    std::string department;
};

struct VendorOffer {
    std::string vendor;
    Money unitPrice;
};

struct PurchaseRecord {
    int orderNo;
    std::string resourceId;
    std::string title;
    ResourceCategory category;
    int quantity;

    Money preTaxCost;
    Money tax;
    Money postTaxCost;
    Money cost;

    bool approved;
    std::string reason;

    bool cancelled = false;
    std::string department;
    std::string vendor;
};

class AcquisitionManager {
public:
    AcquisitionManager(Catalog& catalog, Budget& budget);

    Money quote(const std::string& id, int quantity) const;

    void setPrintTaxRate(double percent);
    void setElectronicTaxRate(double percent);

    double printTaxRate() const {
        return printTaxRate_;
    }

    double electronicTaxRate() const {
        return electronicTaxRate_;
    }

    // Q9: Department budgets.
    void addDepartment(
        const std::string& department,
        Money budget
    );

    Budget& departmentBudget(const std::string& department);
    const Budget& departmentBudget(const std::string& department) const;
    bool hasDepartment(const std::string& department) const;

    // Q12: Vendor offers.
    void addVendorOffer(
        const std::string& resourceId,
        const std::string& vendor,
        Money unitPrice
    );

    VendorOffer cheapestVendor(
        const std::string& resourceId
    ) const;

    // Purchase checking.
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

    // Purchasing.
    const PurchaseRecord& purchase(
        const std::string& id,
        int quantity
    );

    const PurchaseRecord& purchase(
        const std::string& department,
        const std::string& id,
        int quantity
    );

    // Q8: Cancellation.
    const PurchaseRecord& cancel(int orderNo);

    // Q11: Batch processing.
    std::vector<PurchaseRecord> processBatch(
        const std::vector<PurchaseRequest>& reqs,
        bool allOrNothing = false
    );

    const std::vector<PurchaseRecord>& history() const {
        return history_;
    }

    Money totalSpent() const;
    void printReport(std::ostream& os) const;

private:
    Money taxFor(
        const Resource& r,
        Money preTaxCost
    ) const;

    // Q12: Use the cheapest registered vendor's price.
    // If no vendor offers exist, use the catalogue price.
    Money costFor(
        const Resource& resource,
        const std::string& resourceId,
        int quantity,
        std::string* selectedVendor = nullptr
    ) const;

    Budget& budgetFor(const std::string& department);
    const Budget& budgetFor(const std::string& department) const;

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
        const std::string& department = {},
        const std::string& vendor = {}
    );

    Catalog& catalog_;
    Budget& budget_;

    std::map<std::string, Budget> departmentBudgets_;

    // Q12: Resource ID -> registered vendor offers.
    std::map<std::string, std::vector<VendorOffer>> vendorOffers_;

    std::vector<PurchaseRecord> history_;

    int nextOrderNo_ = 1;
    double printTaxRate_ = 0.0;
    double electronicTaxRate_ = 0.0;
};

}  // namespace bookmgmt