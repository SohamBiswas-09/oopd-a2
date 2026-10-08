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
    std::string reason;

    // Q8:
    // True when this history entry represents a cancellation.
    bool cancelled = false;
};

class AcquisitionManager {
public:
    AcquisitionManager(
        Catalog& catalog,
        Budget& budget
    );

    // Price of a request before tax.
    // Throws NotFoundError.
    Money quote(
        const std::string& id,
        int quantity
    ) const;

    // Q6: configurable tax rates.
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

    // Checks whether a purchase would be approved.
    //
    // Q7: the resource title is passed to Budget so that
    // the different-title quota can be checked.
    bool canPurchase(
        const std::string& id,
        int quantity,
        std::string* reason = nullptr
    ) const;

    // Buys immediately.
    //
    // Throws:
    //   NotFoundError
    //   QuotaExceededError
    //   BudgetExceededError
    //   std::invalid_argument
    const PurchaseRecord& purchase(
        const std::string& id,
        int quantity
    );

    // Q8:
    // Cancels an approved purchase order.
    //
    // The original order remains in history and a separate
    // cancellation record is added.
    //
    // Throws std::invalid_argument if the order does not exist,
    // was not approved, or was already cancelled.
    const PurchaseRecord& cancel(int orderNo);

    // Processes requests in order.
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
    // Calculates tax for a resource.
    Money taxFor(
        const Resource& r,
        Money preTaxCost
    ) const;

    // Creates a purchase-history record.
    PurchaseRecord& record(
        const Resource* r,
        const std::string& id,
        int qty,
        Money preTaxCost,
        Money tax,
        Money postTaxCost,
        bool approved,
        std::string reason,
        bool cancelled = false
    );

    Catalog& catalog_;
    Budget& budget_;

    std::vector<PurchaseRecord> history_;

    int nextOrderNo_ = 1;

    // Q6:
    // Configurable tax rates, expressed as percentages.
    double printTaxRate_ = 0.0;
    double electronicTaxRate_ = 0.0;
};

}  // namespace bookmgmt
