#include "bookmgmt/Acquisition.h"

#include <cstdint>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <utility>

#include "bookmgmt/Exceptions.h"

namespace bookmgmt {

// ======================================================
// Constructor
// ======================================================

AcquisitionManager::AcquisitionManager(
    Catalog& catalog,
    Budget& budget)
    : catalog_(catalog),
      budget_(budget) {
}


// ======================================================
// Quote
// ======================================================

Money AcquisitionManager::quote(
    const std::string& id,
    int quantity) const {

    return catalog_.get(id).costFor(quantity);
}


// ======================================================
// Q6: Tax configuration
// ======================================================

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


// ======================================================
// Q6: Calculate tax
// ======================================================

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


// ======================================================
// Q9: Department management
// ======================================================

void AcquisitionManager::addDepartment(
    const std::string& department,
    Money budget) {

    if (department.empty()) {
        throw std::invalid_argument(
            "department name must not be empty"
        );
    }

    if (departmentBudgets_.find(department)
        != departmentBudgets_.end()) {

        throw std::invalid_argument(
            "department already exists: "
            + department
        );
    }

    departmentBudgets_.emplace(
        department,
        Budget(budget)
    );
}


bool AcquisitionManager::hasDepartment(
    const std::string& department) const {

    return departmentBudgets_.find(department)
        != departmentBudgets_.end();
}


Budget& AcquisitionManager::departmentBudget(
    const std::string& department) {

    auto it =
        departmentBudgets_.find(department);

    if (it == departmentBudgets_.end()) {
        throw std::invalid_argument(
            "department not found: "
            + department
        );
    }

    return it->second;
}


const Budget& AcquisitionManager::departmentBudget(
    const std::string& department) const {

    auto it =
        departmentBudgets_.find(department);

    if (it == departmentBudgets_.end()) {
        throw std::invalid_argument(
            "department not found: "
            + department
        );
    }

    return it->second;
}


// ======================================================
// Q9: Select the correct budget
// ======================================================

Budget& AcquisitionManager::budgetFor(
    const std::string& department) {

    if (department.empty()) {
        return budget_;
    }

    return departmentBudget(department);
}


const Budget& AcquisitionManager::budgetFor(
    const std::string& department) const {

    if (department.empty()) {
        return budget_;
    }

    return departmentBudget(department);
}


// ======================================================
// Q7: Check purchase using default budget
// ======================================================

bool AcquisitionManager::canPurchase(
    const std::string& id,
    int quantity,
    std::string* reason) const {

    return canPurchase(
        "",
        id,
        quantity,
        reason
    );
}


// ======================================================
// Q9: Check purchase using department budget
// ======================================================

bool AcquisitionManager::canPurchase(
    const std::string& department,
    const std::string& id,
    int quantity,
    std::string* reason) const {

    std::string why;

    // --------------------------------------------------
    // Find resource.
    // --------------------------------------------------

    const Resource* r =
        catalog_.find(id);

    if (!r) {

        why =
            "resource not found: "
            + id;

    } else if (quantity <= 0) {

        why =
            "quantity must be positive";

    } else {

        // --------------------------------------------------
        // Calculate pre-tax cost.
        // --------------------------------------------------

        const Money preTaxCost =
            r->costFor(quantity);

        // --------------------------------------------------
        // Calculate tax.
        // --------------------------------------------------

        const Money tax =
            taxFor(
                *r,
                preTaxCost
            );

        // --------------------------------------------------
        // Calculate final cost.
        // --------------------------------------------------

        const Money postTaxCost =
            preTaxCost + tax;

        // --------------------------------------------------
        // Check the selected budget.
        //
        // Q6:
        //   Use post-tax cost.
        //
        // Q7:
        //   Pass the resource title.
        //
        // Q9:
        //   Use the department's budget.
        // --------------------------------------------------

        try {

            const Budget& selectedBudget =
                budgetFor(department);

            why =
                selectedBudget.check(
                    r->category(),
                    quantity,
                    postTaxCost,
                    r->title()
                );

        } catch (const std::invalid_argument& e) {

            why = e.what();
        }
    }

    if (reason) {
        *reason = why;
    }

    return why.empty();
}


// ======================================================
// Create purchase-history record
// ======================================================

PurchaseRecord& AcquisitionManager::record(
    const Resource* r,
    const std::string& id,
    int qty,
    Money preTaxCost,
    Money tax,
    Money postTaxCost,
    bool approved,
    std::string reason,
    bool cancelled,
    const std::string& department) {

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

            cancelled,

            department
        }
    );

    return history_.back();
}


// ======================================================
// Original Q1-Q8 purchase API
// Uses the default budget.
// ======================================================

const PurchaseRecord& AcquisitionManager::purchase(
    const std::string& id,
    int quantity) {

    return purchase(
        "",
        id,
        quantity
    );
}


// ======================================================
// Q9: Department purchase
// ======================================================

const PurchaseRecord& AcquisitionManager::purchase(
    const std::string& department,
    const std::string& id,
    int quantity) {

    // --------------------------------------------------
    // Find the resource.
    // --------------------------------------------------

    const Resource& r =
        catalog_.get(id);

    // --------------------------------------------------
    // Calculate original price.
    // --------------------------------------------------

    const Money preTaxCost =
        r.costFor(quantity);

    // --------------------------------------------------
    // Calculate tax.
    // --------------------------------------------------

    const Money tax =
        taxFor(
            r,
            preTaxCost
        );

    // --------------------------------------------------
    // Calculate final amount.
    // --------------------------------------------------

    const Money postTaxCost =
        preTaxCost + tax;

    // --------------------------------------------------
    // Select the correct budget.
    // --------------------------------------------------

    Budget& selectedBudget =
        budgetFor(department);

    // --------------------------------------------------
    // Commit against the selected budget.
    //
    // This checks:
    //   1. unit quota
    //   2. spend quota
    //   3. overall budget
    //   4. different-title quota
    //
    // If the check fails, Budget::commit throws and
    // nothing is changed.
    // --------------------------------------------------

    selectedBudget.commit(
        r.category(),
        quantity,
        postTaxCost,
        r.title()
    );

    // --------------------------------------------------
    // Update catalogue holdings.
    // --------------------------------------------------

    catalog_.addHoldings(
        id,
        quantity
    );

    // --------------------------------------------------
    // Add purchase to history.
    // --------------------------------------------------

    return record(
        &r,
        id,
        quantity,
        preTaxCost,
        tax,
        postTaxCost,
        true,
        {},
        false,
        department
    );
}


// ======================================================
// Q8: Cancellation
// ======================================================

const PurchaseRecord& AcquisitionManager::cancel(
    int orderNo) {

    // --------------------------------------------------
    // Find the original order.
    // --------------------------------------------------

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

    // --------------------------------------------------
    // Only approved purchases can be cancelled.
    // --------------------------------------------------

    if (!original->approved) {

        throw std::invalid_argument(
            "order is not an approved purchase"
        );
    }

    // --------------------------------------------------
    // The same order cannot be cancelled twice.
    // --------------------------------------------------

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

    // --------------------------------------------------
    // Save the original information before adding the
    // cancellation record.
    //
    // push_back() may reallocate history_, so we must
    // not keep using the original pointer afterwards.
    // --------------------------------------------------

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

    const std::string department =
        original->department;

    // --------------------------------------------------
    // Select the budget that originally paid for the
    // purchase.
    // --------------------------------------------------

    Budget& selectedBudget =
        budgetFor(department);

    // --------------------------------------------------
    // Refund:
    //   - units
    //   - category spending
    //   - title usage
    //   - overall spending
    // --------------------------------------------------

    selectedBudget.refund(
        category,
        quantity,
        postTaxCost,
        title
    );

    // --------------------------------------------------
    // Reduce catalogue holdings.
    // --------------------------------------------------

    catalog_.addHoldings(
        id,
        -quantity
    );

    // --------------------------------------------------
    // Find the resource again for the cancellation
    // record.
    // --------------------------------------------------

    Resource* resource =
        catalog_.find(id);

    // --------------------------------------------------
    // Keep the original purchase record unchanged.
    // Add a separate cancellation record.
    // --------------------------------------------------

    return record(
        resource,
        id,
        quantity,
        preTaxCost,
        tax,
        postTaxCost,
        false,
        cancellationMarker,
        true,
        department
    );
}


// ======================================================
// Q8 + Q9 + Q11: Batch processing
// ======================================================
//
// allOrNothing = false:
//     Existing behavior. Requests are processed
//     independently.
//
// allOrNothing = true:
//     First simulate the entire batch using copies of
//     the budgets. No real purchase is performed during
//     validation.
//
//     If any request fails:
//         - no real budget is changed
//         - no catalogue holdings are changed
//         - no request is purchased
//
//     If every request succeeds:
//         - the real purchase() function is called for
//           every request.
// ======================================================

std::vector<PurchaseRecord>
AcquisitionManager::processBatch(
    const std::vector<PurchaseRequest>& reqs,
    bool allOrNothing) {

    std::vector<PurchaseRecord> results;

    results.reserve(
        reqs.size()
    );

    // ==================================================
    // Q11: All-or-nothing mode
    // ==================================================

    if (allOrNothing) {

        // ------------------------------------------------
        // Create temporary copies of all budgets.
        //
        // These copies are used only for simulation.
        // The real budgets are not modified until the
        // complete batch has passed validation.
        // ------------------------------------------------

        Budget temporaryDefaultBudget =
            budget_;

        std::map<std::string, Budget>
            temporaryDepartmentBudgets =
                departmentBudgets_;

        // ------------------------------------------------
        // Store the calculated information for each
        // request so we do not need to calculate it
        // again after validation.
        // ------------------------------------------------

        struct BatchItem {
            const Resource* resource = nullptr;
            Money preTaxCost;
            Money tax;
            Money postTaxCost;
            std::string reason;
        };

        std::vector<BatchItem> items;

        items.reserve(
            reqs.size()
        );

        bool batchValid = true;

        // ------------------------------------------------
        // Preflight every request.
        // ------------------------------------------------

        for (const auto& req : reqs) {

            BatchItem item;

            // --------------------------------------------
            // Find resource.
            // --------------------------------------------

            item.resource =
                catalog_.find(
                    req.resourceId
                );

            if (!item.resource) {

                item.reason =
                    "resource not found: "
                    + req.resourceId;

                batchValid = false;

                items.push_back(
                    std::move(item)
                );

                continue;
            }

            // --------------------------------------------
            // Check quantity.
            // --------------------------------------------

            if (req.quantity <= 0) {

                item.reason =
                    "quantity must be positive";

                batchValid = false;

                items.push_back(
                    std::move(item)
                );

                continue;
            }

            // --------------------------------------------
            // Calculate pre-tax cost.
            // --------------------------------------------

            item.preTaxCost =
                item.resource->costFor(
                    req.quantity
                );

            // --------------------------------------------
            // Calculate tax.
            // --------------------------------------------

            item.tax =
                taxFor(
                    *item.resource,
                    item.preTaxCost
                );

            // --------------------------------------------
            // Calculate final post-tax cost.
            // --------------------------------------------

            item.postTaxCost =
                item.preTaxCost + item.tax;

            // --------------------------------------------
            // Select the temporary budget.
            // --------------------------------------------

            try {

                Budget* selectedBudget = nullptr;

                if (req.department.empty()) {

                    selectedBudget =
                        &temporaryDefaultBudget;

                } else {

                    auto it =
                        temporaryDepartmentBudgets.find(
                            req.department
                        );

                    if (it ==
                        temporaryDepartmentBudgets.end()) {

                        throw std::invalid_argument(
                            "department not found: "
                            + req.department
                        );
                    }

                    selectedBudget =
                        &it->second;
                }

                // ----------------------------------------
                // Check the request against the temporary
                // budget.
                //
                // This is important because earlier
                // requests in the same batch have already
                // been committed to the temporary budget.
                // ----------------------------------------

                item.reason =
                    selectedBudget->check(
                        item.resource->category(),
                        req.quantity,
                        item.postTaxCost,
                        item.resource->title()
                    );

                // ----------------------------------------
                // If the request passes, commit it to the
                // temporary budget.
                //
                // This does NOT change the real budget.
                // ----------------------------------------

                if (item.reason.empty()) {

                    selectedBudget->commit(
                        item.resource->category(),
                        req.quantity,
                        item.postTaxCost,
                        item.resource->title()
                    );
                }

            } catch (const std::invalid_argument& e) {

                item.reason = e.what();
            }

            if (!item.reason.empty()) {
                batchValid = false;
            }

            items.push_back(
                std::move(item)
            );
        }

        // =================================================
        // The complete batch failed.
        // =================================================

        if (!batchValid) {

            for (std::size_t i = 0;
                 i < reqs.size();
                 ++i) {

                const auto& req =
                    reqs[i];

                const auto& item =
                    items[i];

                std::string reason;

                if (!item.reason.empty()) {

                    reason =
                        item.reason;

                } else {

                    reason =
                        "batch rejected: "
                        "all-or-nothing validation failed";
                }

                results.push_back(
                    record(
                        item.resource,
                        req.resourceId,
                        req.quantity,
                        item.preTaxCost,
                        item.tax,
                        item.postTaxCost,
                        false,
                        reason,
                        false,
                        req.department
                    )
                );
            }

            return results;
        }

        // =================================================
        // The complete batch passed validation.
        //
        // Now perform the real purchases.
        // =================================================

        for (const auto& req : reqs) {

            results.push_back(
                purchase(
                    req.department,
                    req.resourceId,
                    req.quantity
                )
            );
        }

        return results;
    }

    // ==================================================
    // Original behavior
    //
    // allOrNothing == false
    //
    // Each request is processed independently.
    // ==================================================

    for (const auto& req : reqs) {

        const Resource* r =
            catalog_.find(
                req.resourceId
            );

        Money preTaxCost;
        Money tax;
        Money postTaxCost;

        std::string why;

        // --------------------------------------------------
        // Find resource.
        // --------------------------------------------------

        if (!r) {

            why =
                "resource not found: "
                + req.resourceId;

        } else if (req.quantity <= 0) {

            why =
                "quantity must be positive";

        } else {

            // --------------------------------------------------
            // Calculate tax breakdown.
            // --------------------------------------------------

            preTaxCost =
                r->costFor(
                    req.quantity
                );

            tax =
                taxFor(
                    *r,
                    preTaxCost
                );

            postTaxCost =
                preTaxCost + tax;

            // --------------------------------------------------
            // Check the correct department budget.
            // --------------------------------------------------

            try {

                const Budget& selectedBudget =
                    budgetFor(
                        req.department
                    );

                why =
                    selectedBudget.check(
                        r->category(),
                        req.quantity,
                        postTaxCost,
                        r->title()
                    );

            } catch (const std::invalid_argument& e) {

                why = e.what();
            }
        }

        // --------------------------------------------------
        // Approved request.
        // --------------------------------------------------

        if (why.empty()) {

            results.push_back(
                purchase(
                    req.department,
                    req.resourceId,
                    req.quantity
                )
            );

        } else {

            // --------------------------------------------------
            // Rejected requests are still recorded.
            // --------------------------------------------------

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
                    false,
                    req.department
                )
            );
        }
    }

    return results;
}


// ======================================================
// Total spent
// ======================================================

Money AcquisitionManager::totalSpent() const {

    Money sum;

    for (const auto& rec : history_) {

        // --------------------------------------------------
        // Cancellation records are not spending.
        //
        // Also skip an approved order when a corresponding
        // cancellation record exists.
        // --------------------------------------------------

        if (!rec.approved) {
            continue;
        }

        const std::string cancellationMarker =
            "Cancellation of order #"
            + std::to_string(rec.orderNo);

        bool cancelled = false;

        for (const auto& other : history_) {

            if (other.cancelled &&
                other.reason == cancellationMarker) {

                cancelled = true;
                break;
            }
        }

        if (!cancelled) {
            sum += rec.postTaxCost;
        }
    }

    return sum;
}


// ======================================================
// Print acquisition report
// ======================================================

void AcquisitionManager::printReport(
    std::ostream& os) const {

    os << "Order history ("
       << history_.size()
       << " records):\n";

    for (const auto& rec : history_) {

        os << "Order #"
           << rec.orderNo
           << " | ";

        // --------------------------------------------------
        // Department information.
        // --------------------------------------------------

        if (!rec.department.empty()) {

            os << "Department: "
               << rec.department
               << " | ";
        }

        os << rec.title
           << " | quantity: "
           << rec.quantity
           << " | pre-tax: "
           << rec.preTaxCost
           << " | tax: "
           << rec.tax
           << " | post-tax: "
           << rec.postTaxCost
           << " | ";

        // --------------------------------------------------
        // Status.
        // --------------------------------------------------

        if (rec.cancelled) {

            os << "CANCELLED";

        } else if (rec.approved) {

            os << "APPROVED";

        } else {

            os << "REJECTED";
        }

        // --------------------------------------------------
        // Rejection/cancellation reason.
        // --------------------------------------------------

        if (!rec.reason.empty()) {

            os << " | reason: "
               << rec.reason;
        }

        os << "\n";
    }

    os << "Total spent (post-tax): "
       << totalSpent()
       << "\n";
}

}  // namespace bookmgmt
