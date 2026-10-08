#include "bookmgmt/Acquisition.h"

#include <cstdint>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "bookmgmt/Exceptions.h"

namespace bookmgmt {

AcquisitionManager::AcquisitionManager(
    Catalog& catalog,
    Budget& budget)
    : catalog_(catalog),
      budget_(budget) {
}

Money AcquisitionManager::quote(
    const std::string& id,
    int quantity) const {

    return catalog_.get(id).costFor(quantity);
}

void AcquisitionManager::setPrintTaxRate(
    double percent) {

    if (percent < 0.0) {
        throw std::invalid_argument(
            "tax rate must not be negative"
        );
    }

    printTaxRate_ = percent;
}

void AcquisitionManager::setElectronicTaxRate(
    double percent) {

    if (percent < 0.0) {
        throw std::invalid_argument(
            "tax rate must not be negative"
        );
    }

    electronicTaxRate_ = percent;
}

Money AcquisitionManager::taxFor(
    const Resource& r,
    Money preTaxCost) const {

    const double rate =
        r.isDigital()
            ? electronicTaxRate_
            : printTaxRate_;

    const std::int64_t taxMinorUnits =
        static_cast<std::int64_t>(
            preTaxCost.minorUnits()
            * rate
            / 100.0
        );

    return Money::fromMinor(
        taxMinorUnits
    );
}

bool AcquisitionManager::canPurchase(
    const std::string& id,
    int quantity,
    std::string* reason) const {

    std::string why;

    if (const Resource* r = catalog_.find(id)) {

        if (quantity <= 0) {

            why = "quantity must be positive";

        } else {

            // ------------------------------------------
            // Calculate pre-tax cost.
            // ------------------------------------------

            const Money preTaxCost =
                r->costFor(quantity);

            // ------------------------------------------
            // Calculate tax.
            // ------------------------------------------

            const Money tax =
                taxFor(*r, preTaxCost);

            // ------------------------------------------
            // Calculate final cost.
            // ------------------------------------------

            const Money postTaxCost =
                preTaxCost + tax;

            // ------------------------------------------
            // Q6 + Q7:
            //
            // Budget checks:
            //   - post-tax cost
            //   - resource title
            // ------------------------------------------

            why = budget_.check(
                r->category(),
                quantity,
                postTaxCost,
                r->title()
            );
        }

    } else {

        why =
            "resource not found: " + id;
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
    std::string reason,
    bool cancelled) {

    history_.push_back(
        PurchaseRecord{
            nextOrderNo_++,
            id,
            r
                ? r->title()
                : std::string("(unknown)"),
            r
                ? r->category()
                : ResourceCategory::Book,
            qty,

            preTaxCost,
            tax,
            postTaxCost,

            // Backward compatibility:
            // cost represents the final amount charged.
            postTaxCost,

            approved,
            std::move(reason),
            cancelled
        }
    );

    return history_.back();
}

const PurchaseRecord& AcquisitionManager::purchase(
    const std::string& id,
    int quantity) {

    // ----------------------------------------------
    // Find the resource.
    // ----------------------------------------------

    const Resource& r =
        catalog_.get(id);

    // ----------------------------------------------
    // Calculate original price.
    // ----------------------------------------------

    const Money preTaxCost =
        r.costFor(quantity);

    // ----------------------------------------------
    // Calculate tax.
    // ----------------------------------------------

    const Money tax =
        taxFor(r, preTaxCost);

    // ----------------------------------------------
    // Calculate final amount.
    // ----------------------------------------------

    const Money postTaxCost =
        preTaxCost + tax;

    // ----------------------------------------------
    // Q6 + Q7:
    //
    // Check the budget using:
    //   - post-tax cost
    //   - resource title
    //
    // If this fails, nothing below is changed.
    // ----------------------------------------------

    budget_.commit(
        r.category(),
        quantity,
        postTaxCost,
        r.title()
    );

    // ----------------------------------------------
    // Update catalogue holdings.
    // ----------------------------------------------

    catalog_.addHoldings(
        id,
        quantity
    );

    // ----------------------------------------------
    // Add purchase to history.
    // ----------------------------------------------

    return record(
        &r,
        id,
        quantity,
        preTaxCost,
        tax,
        postTaxCost,
        true,
        {},
        false
    );
}

// ------------------------------------------------------
// Q8: Cancel an approved order.
// ------------------------------------------------------

const PurchaseRecord& AcquisitionManager::cancel(
    int orderNo) {

    // ----------------------------------------------
    // Find the original order.
    // ----------------------------------------------

    PurchaseRecord* original = nullptr;

    for (auto& rec : history_) {

        if (rec.orderNo == orderNo) {
            original = &rec;
            break;
        }
    }

    if (!original) {

        throw std::invalid_argument(
            "order not found: "
            + std::to_string(orderNo)
        );
    }

    // ----------------------------------------------
    // Only approved purchases can be cancelled.
    // ----------------------------------------------

    if (!original->approved) {

        throw std::invalid_argument(
            "order is not an approved purchase"
        );
    }

    // ----------------------------------------------
    // Prevent duplicate cancellation.
    //
    // The cancellation record stores the original
    // order number in its reason.
    // ----------------------------------------------

    const std::string cancellationMarker =
        "Cancellation of order #"
        + std::to_string(orderNo);

    for (const auto& rec : history_) {

        if (rec.cancelled &&
            rec.reason == cancellationMarker) {

            throw std::invalid_argument(
                "order already cancelled: "
                + std::to_string(orderNo)
            );
        }
    }

    // ----------------------------------------------
    // Save the original order information before
    // adding the cancellation record.
    //
    // push_back() may reallocate history_, so we
    // must not keep using the original pointer after
    // adding a new record.
    // ----------------------------------------------

    const std::string id =
        original->resourceId;

    const std::string title =
        original->title;

    const ResourceCategory category =
        original->category;

    const int quantity =
        original->quantity;

    const Money preTaxCost =
        original->preTaxCost;

    const Money tax =
        original->tax;

    const Money postTaxCost =
        original->postTaxCost;

    // ----------------------------------------------
    // Refund budget and quota usage.
    // ----------------------------------------------

    budget_.refund(
        category,
        quantity,
        postTaxCost,
        title
    );

    // ----------------------------------------------
    // Reduce catalogue holdings.
    // ----------------------------------------------

    catalog_.addHoldings(
        id,
        -quantity
    );

    // ----------------------------------------------
    // Keep the original purchase record unchanged.
    //
    // Add a separate cancellation record.
    // ----------------------------------------------

    Resource* resource =
        catalog_.find(id);

    return record(
        resource,
        id,
        quantity,
        preTaxCost,
        tax,
        postTaxCost,
        false,
        cancellationMarker,
        true
    );
}

std::vector<PurchaseRecord>
AcquisitionManager::processBatch(
    const std::vector<PurchaseRequest>& reqs) {

    std::vector<PurchaseRecord> results;

    results.reserve(reqs.size());

    for (const auto& req : reqs) {

        const Resource* r =
            catalog_.find(req.resourceId);

        Money preTaxCost;
        Money tax;
        Money postTaxCost;

        std::string why;

        if (!r) {

            why =
                "resource not found: "
                + req.resourceId;

        } else if (req.quantity <= 0) {

            why =
                "quantity must be positive";

        } else {

            // --------------------------------------
            // Calculate tax breakdown.
            // --------------------------------------

            preTaxCost =
                r->costFor(req.quantity);

            tax =
                taxFor(*r, preTaxCost);

            postTaxCost =
                preTaxCost + tax;

            // --------------------------------------
            // Q6 + Q7:
            //
            // Check using post-tax cost and title.
            // --------------------------------------

            why = budget_.check(
                r->category(),
                req.quantity,
                postTaxCost,
                r->title()
            );
        }

        if (why.empty()) {

            // purchase() performs the actual update.
            results.push_back(
                purchase(
                    req.resourceId,
                    req.quantity
                )
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
                    why,
                    false
                )
            );
        }
    }

    return results;
}

// ------------------------------------------------------
// Calculate total active spending.
//
// The original purchase record remains in history after
// cancellation. Therefore, we check whether a separate
// cancellation record refers to that purchase before
// adding its cost.
// ------------------------------------------------------

Money AcquisitionManager::totalSpent() const {

    Money sum;

    for (const auto& rec : history_) {

        // Rejected purchases and cancellation records
        // are not active spending.
        if (!rec.approved || rec.cancelled) {
            continue;
        }

        const std::string cancellationMarker =
            "Cancellation of order #"
            + std::to_string(rec.orderNo);

        bool wasCancelled = false;

        for (const auto& cancellation : history_) {

            if (cancellation.cancelled &&
                cancellation.reason ==
                    cancellationMarker) {

                wasCancelled = true;
                break;
            }
        }

        if (!wasCancelled) {
            sum += rec.postTaxCost;
        }
    }

    return sum;
}

void AcquisitionManager::printReport(
    std::ostream& os) const {

    os << "Order history ("
       << history_.size()
       << " orders)\n";

    for (const auto& rec : history_) {

        os << "  #"
           << std::setw(3)
           << std::left
           << rec.orderNo
           << " ";

        if (rec.cancelled) {

            os << "CANCELLED";

        } else if (rec.approved) {

            os << "APPROVED";

        } else {

            os << "REJECTED";
        }

        os << "  "
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

        if (!rec.approved || rec.cancelled) {

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
