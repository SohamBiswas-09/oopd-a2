#include "bookmgmt/Acquisition.h"

#include <cstdint>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "bookmgmt/Exceptions.h"

namespace bookmgmt {

AcquisitionManager::AcquisitionManager(Catalog& catalog, Budget& budget)
    : catalog_(catalog), budget_(budget) {}

Money AcquisitionManager::quote(const std::string& id, int quantity) const {
    return catalog_.get(id).costFor(quantity);
}

void AcquisitionManager::setPrintTaxRate(double percent) {
    if (percent < 0.0) {
        throw std::invalid_argument(
            "tax rate must not be negative");
    }

    printTaxRate_ = percent;
}

void AcquisitionManager::setElectronicTaxRate(double percent) {
    if (percent < 0.0) {
        throw std::invalid_argument(
            "tax rate must not be negative");
    }

    electronicTaxRate_ = percent;
}

Money AcquisitionManager::taxFor(const Resource& r,
                                  Money preTaxCost) const {
    const double rate = r.isDigital()
                           ? electronicTaxRate_
                           : printTaxRate_;

    const std::int64_t taxMinorUnits =
        static_cast<std::int64_t>(
            preTaxCost.minorUnits() * rate / 100.0);

    return Money::fromMinor(taxMinorUnits);
}

bool AcquisitionManager::canPurchase(const std::string& id,
                                     int quantity,
                                     std::string* reason) const {
    std::string why;

    if (const Resource* r = catalog_.find(id)) {
        if (quantity <= 0) {
            why = "quantity must be positive";
        } else {
            const Money preTaxCost = r->costFor(quantity);
            const Money tax = taxFor(*r, preTaxCost);
            const Money postTaxCost = preTaxCost + tax;

            // Q6: check the budget and quotas against
            // the post-tax cost.
            why = budget_.check(
                r->category(),
                quantity,
                postTaxCost
            );
        }
    } else {
        why = "resource not found: " + id;
    }

    if (reason) {
        *reason = why;
    }

    return why.empty();
}

PurchaseRecord& AcquisitionManager::record(
    const Resource* r,
    const std::string& id,
    int qty,
    Money preTaxCost,
    Money tax,
    Money postTaxCost,
    bool approved,
    std::string reason) {

    history_.push_back(PurchaseRecord{
        nextOrderNo_++,
        id,
        r ? r->title() : std::string("(unknown)"),
        r ? r->category() : ResourceCategory::Book,
        qty,

        // Q6 tax breakdown.
        preTaxCost,
        tax,
        postTaxCost,

        // Keep the original `cost` field compatible with
        // previous code. It represents the final amount charged.
        postTaxCost,

        approved,
        std::move(reason)
    });

    return history_.back();
}

const PurchaseRecord& AcquisitionManager::purchase(
    const std::string& id,
    int quantity) {

    const Resource& r = catalog_.get(id);

    // Step 1: calculate the original resource price.
    const Money preTaxCost = r.costFor(quantity);

    // Step 2: calculate tax.
    const Money tax = taxFor(r, preTaxCost);

    // Step 3: calculate the final amount.
    const Money postTaxCost = preTaxCost + tax;

    // Step 4: Q6 requires quota and budget checks
    // to use the post-tax cost.
    budget_.commit(
        r.category(),
        quantity,
        postTaxCost
    );

    // Step 5: update catalogue holdings only after
    // the budget successfully accepts the purchase.
    catalog_.addHoldings(id, quantity);

    // Step 6: record the complete purchase information.
    return record(
        &r,
        id,
        quantity,
        preTaxCost,
        tax,
        postTaxCost,
        true,
        {}
    );
}

std::vector<PurchaseRecord> AcquisitionManager::processBatch(
    const std::vector<PurchaseRequest>& reqs) {

    std::vector<PurchaseRecord> results;
    results.reserve(reqs.size());

    for (const auto& req : reqs) {
        const Resource* r = catalog_.find(req.resourceId);

        Money preTaxCost;
        Money tax;
        Money postTaxCost;
        std::string why;

        if (!r) {
            why = "resource not found: " + req.resourceId;
        } else if (req.quantity <= 0) {
            why = "quantity must be positive";
        } else {
            // Calculate the complete tax breakdown.
            preTaxCost = r->costFor(req.quantity);
            tax = taxFor(*r, preTaxCost);
            postTaxCost = preTaxCost + tax;

            // Q6: quota and budget checks use post-tax cost.
            why = budget_.check(
                r->category(),
                req.quantity,
                postTaxCost
            );
        }

        if (why.empty()) {
            // purchase() recalculates the same values and performs
            // the actual budget/holdings update.
            results.push_back(
                purchase(req.resourceId, req.quantity)
            );
        } else {
            // Rejected requests are still recorded.
            results.push_back(
                record(
                    r,
                    req.resourceId,
                    req.quantity,
                    preTaxCost,
                    tax,
                    postTaxCost,
                    false,
                    why
                )
            );
        }
    }

    return results;
}

Money AcquisitionManager::totalSpent() const {
    Money sum;

    for (const auto& rec : history_) {
        if (rec.approved) {
            // Actual spending includes tax.
            sum += rec.postTaxCost;
        }
    }

    return sum;
}

void AcquisitionManager::printReport(std::ostream& os) const {
    os << "Order history (" << history_.size()
       << " orders)\n";

    for (const auto& rec : history_) {
        os << "  #"
           << std::setw(3)
           << std::left
           << rec.orderNo
           << " "
           << (rec.approved ? "APPROVED" : "REJECTED")
           << "  "
           << std::setw(6)
           << rec.resourceId
           << " x"
           << std::setw(3)
           << rec.quantity
           << "  pre-tax: "
           << rec.preTaxCost
           << "  tax: "
           << rec.tax
           << "  post-tax: "
           << rec.postTaxCost
           << "  "
           << rec.title;

        if (!rec.approved) {
            os << "\n        reason: "
               << rec.reason;
        }

        os << "\n";
    }

    os << "Total spent (post-tax): "
       << totalSpent()
       << "\n";
}

}  // namespace bookmgmt
