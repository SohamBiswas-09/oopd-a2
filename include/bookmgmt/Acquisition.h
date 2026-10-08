#pragma once
// AcquisitionManager: turns purchase requests into orders, enforcing the
// Budget's quotas, updating Catalog holdings and keeping an order history.

#include <iosfwd>
#include <string>
#include <vector>

#include "bookmgmt/Budget.h"
#include "bookmgmt/Catalog.h"

namespace bookmgmt {

struct PurchaseRequest {
    std::string resourceId;
    int quantity;  // copies for print, seats for electronic
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
    std::string reason;  // why it was rejected; empty if approved
};

class AcquisitionManager {
public:
    AcquisitionManager(Catalog& catalog, Budget& budget);

    // Price of a request before tax. Throws NotFoundError.
    Money quote(const std::string& id, int quantity) const;

    // Q6: configurable tax rates.
    // Rates are percentages, so 5.0 means 5%.
    void setPrintTaxRate(double percent);
    void setElectronicTaxRate(double percent);

    double printTaxRate() const {
        return printTaxRate_;
    }

    double electronicTaxRate() const {
        return electronicTaxRate_;
    }

    // True if the purchase would be approved; if not, `reason` explains why.
    // Q6: budget/quota checks use the post-tax cost.
    bool canPurchase(const std::string& id, int quantity,
                     std::string* reason = nullptr) const;

    // Buys immediately. Throws NotFoundError, QuotaExceededError,
    // BudgetExceededError or std::invalid_argument. On success the budget
    // and holdings are updated and the record is added to history.
    const PurchaseRecord& purchase(const std::string& id, int quantity);

    // Processes requests in order; each is approved or rejected on its own
    // (never throws for a rejected request). Every outcome is recorded.
    // EXTENSION POINT: priority ordering, all-or-nothing batches, ...
    std::vector<PurchaseRecord> processBatch(
        const std::vector<PurchaseRequest>& reqs);

    const std::vector<PurchaseRecord>& history() const {
        return history_;
    }

    // Q6: total amount actually spent, including tax.
    Money totalSpent() const;

    void printReport(std::ostream& os) const;

private:
    // Calculates tax for a resource using the configured rate.
    Money taxFor(const Resource& r, Money preTaxCost) const;

    // Creates a purchase-history record.
    PurchaseRecord& record(const Resource* r,
                           const std::string& id,
                           int qty,
                           Money preTaxCost,
                           Money tax,
                           Money postTaxCost,
                           bool approved,
                           std::string reason);

    Catalog& catalog_;
    Budget& budget_;

    std::vector<PurchaseRecord> history_;
    int nextOrderNo_ = 1;

    // Q6: configurable tax rates, expressed as percentages.
    double printTaxRate_ = 0.0;
    double electronicTaxRate_ = 0.0;
};

}  // namespace bookmgmt
