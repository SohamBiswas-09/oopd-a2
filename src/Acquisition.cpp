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
// Q12: Vendor offers
// ======================================================

void AcquisitionManager::addVendorOffer(
    const std::string& resourceId,
    const std::string& vendor,
    Money unitPrice) {

    if (!catalog_.contains(resourceId)) {
        throw std::invalid_argument(
            "resource not found: " + resourceId);
    }

    if (vendor.empty()) {
        throw std::invalid_argument(
            "vendor name must not be empty");
    }

    if (unitPrice.isNegative()) {
        throw std::invalid_argument(
            "vendor price must not be negative");
    }

    auto& offers = vendorOffers_[resourceId];

    // A vendor can update its own offer instead of creating duplicates.
    for (auto& offer : offers) {
        if (offer.vendor == vendor) {
            offer.unitPrice = unitPrice;
            return;
        }
    }

    offers.push_back(VendorOffer{vendor, unitPrice});
}

VendorOffer AcquisitionManager::cheapestVendor(
    const std::string& resourceId) const {

    const auto it = vendorOffers_.find(resourceId);

    if (it == vendorOffers_.end() || it->second.empty()) {
        throw std::invalid_argument(
            "no vendor offers for resource: " + resourceId);
    }

    const VendorOffer* cheapest = &it->second.front();

    for (const auto& offer : it->second) {
        if (offer.unitPrice < cheapest->unitPrice) {
            cheapest = &offer;
        }
    }

    // Strictly less-than comparison means equal-price ties
    // are resolved in favor of the first registered offer.
    return *cheapest;
}

Money AcquisitionManager::costFor(
    const Resource& resource,
    const std::string& resourceId,
    int quantity,
    std::string* selectedVendor) const {

    const auto it = vendorOffers_.find(resourceId);

    if (it == vendorOffers_.end() || it->second.empty()) {
        if (selectedVendor) {
            selectedVendor->clear();
        }

        return resource.costFor(quantity);
    }

    const VendorOffer offer = cheapestVendor(resourceId);

    if (selectedVendor) {
        *selectedVendor = offer.vendor;
    }

    return resource.costForAtPrice(quantity, offer.unitPrice);
}

// ======================================================
// Quote
// ======================================================

Money AcquisitionManager::quote(
    const std::string& id,
    int quantity) const {

    const Resource& resource = catalog_.get(id);

    return costFor(resource, id, quantity);
}

// ======================================================
// Q6: Tax configuration
// ======================================================

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
            preTaxCost.minorUnits() * rate / 100.0);

    return Money::fromMinor(taxMinorUnits);
}

// ======================================================
// Q9: Department management
// ======================================================

void AcquisitionManager::addDepartment(
    const std::string& department,
    Money budget) {

    if (department.empty()) {
        throw std::invalid_argument(
            "department name must not be empty");
    }

    if (departmentBudgets_.find(department)
        != departmentBudgets_.end()) {
        throw std::invalid_argument(
            "department already exists: " + department);
    }

    departmentBudgets_.emplace(department, Budget(budget));
}

bool AcquisitionManager::hasDepartment(
    const std::string& department) const {

    return departmentBudgets_.find(department)
        != departmentBudgets_.end();
}

Budget& AcquisitionManager::departmentBudget(
    const std::string& department) {

    auto it = departmentBudgets_.find(department);

    if (it == departmentBudgets_.end()) {
        throw std::invalid_argument(
            "department not found: " + department);
    }

    return it->second;
}

const Budget& AcquisitionManager::departmentBudget(
    const std::string& department) const {

    auto it = departmentBudgets_.find(department);

    if (it == departmentBudgets_.end()) {
        throw std::invalid_argument(
            "department not found: " + department);
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

    return canPurchase("", id, quantity, reason);
}

// ======================================================
// Q9 + Q12: Check purchase using department budget
// ======================================================

bool AcquisitionManager::canPurchase(
    const std::string& department,
    const std::string& id,
    int quantity,
    std::string* reason) const {

    std::string why;
    const Resource* resource = catalog_.find(id);

    if (!resource) {
        why = "resource not found: " + id;
    } else if (quantity <= 0) {
        why = "quantity must be positive";
    } else {
        try {
            const Money preTaxCost =
                costFor(*resource, id, quantity);

            const Money tax =
                taxFor(*resource, preTaxCost);

            const Money postTaxCost = preTaxCost + tax;

            const Budget& selectedBudget = budgetFor(department);

            why = selectedBudget.check(
                resource->category(),
                quantity,
                postTaxCost,
                resource->title());
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
    const std::string& department,
    const std::string& vendor) {

    history_.push_back(
        PurchaseRecord{
            nextOrderNo_++,
            id,
            r ? r->title() : std::string("(unknown)"),
            r ? r->category() : ResourceCategory::Book,
            qty,
            preTaxCost,
            tax,
            postTaxCost,
            postTaxCost,
            approved,
            std::move(reason),
            cancelled,
            department,
            vendor
        });

    return history_.back();
}

// ======================================================
// Original Q1-Q8 purchase API
// ======================================================

const PurchaseRecord& AcquisitionManager::purchase(
    const std::string& id,
    int quantity) {

    return purchase("", id, quantity);
}

// ======================================================
// Q9 + Q12: Department purchase
// ======================================================

const PurchaseRecord& AcquisitionManager::purchase(
    const std::string& department,
    const std::string& id,
    int quantity) {

    const Resource& resource = catalog_.get(id);

    std::string vendor;

    const Money preTaxCost =
        costFor(resource, id, quantity, &vendor);

    const Money tax = taxFor(resource, preTaxCost);
    const Money postTaxCost = preTaxCost + tax;

    Budget& selectedBudget = budgetFor(department);

    selectedBudget.commit(
        resource.category(),
        quantity,
        postTaxCost,
        resource.title());

    catalog_.addHoldings(id, quantity);

    return record(
        &resource,
        id,
        quantity,
        preTaxCost,
        tax,
        postTaxCost,
        true,
        {},
        false,
        department,
        vendor);
}

// ======================================================
// Q8 + Q12: Cancellation
// ======================================================

const PurchaseRecord& AcquisitionManager::cancel(int orderNo) {
    PurchaseRecord* original = nullptr;

    for (auto& rec : history_) {
        if (rec.orderNo == orderNo) {
            original = &rec;
            break;
        }
    }

    if (!original) {
        throw std::invalid_argument(
            "order not found: " + std::to_string(orderNo));
    }

    if (!original->approved) {
        throw std::invalid_argument(
            "order is not an approved purchase");
    }

    const std::string cancellationMarker =
        "Cancellation of order #" + std::to_string(orderNo);

    for (const auto& rec : history_) {
        if (rec.cancelled && rec.reason == cancellationMarker) {
            throw std::invalid_argument(
                "order already cancelled: " + std::to_string(orderNo));
        }
    }

    // Copy all needed values before appending to history_.
    const std::string id = original->resourceId;
    const std::string title = original->title;
    const ResourceCategory category = original->category;
    const int quantity = original->quantity;
    const Money preTaxCost = original->preTaxCost;
    const Money tax = original->tax;
    const Money postTaxCost = original->postTaxCost;
    const std::string department = original->department;
    const std::string vendor = original->vendor;

    Budget& selectedBudget = budgetFor(department);

    selectedBudget.refund(
        category,
        quantity,
        postTaxCost,
        title);

    catalog_.addHoldings(id, -quantity);

    Resource* resource = catalog_.find(id);

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
        department,
        vendor);
}

// ======================================================
// Q8 + Q9 + Q11 + Q12: Batch processing
// ======================================================

std::vector<PurchaseRecord> AcquisitionManager::processBatch(
    const std::vector<PurchaseRequest>& reqs,
    bool allOrNothing) {

    std::vector<PurchaseRecord> results;
    results.reserve(reqs.size());

    // ==================================================
    // Q11: All-or-nothing mode
    // ==================================================

    if (allOrNothing) {
        Budget temporaryDefaultBudget = budget_;
        std::map<std::string, Budget> temporaryDepartmentBudgets =
            departmentBudgets_;

        struct BatchItem {
            const Resource* resource = nullptr;
            Money preTaxCost;
            Money tax;
            Money postTaxCost;
            std::string reason;
            std::string vendor;
        };

        std::vector<BatchItem> items;
        items.reserve(reqs.size());

        bool batchValid = true;

        for (const auto& req : reqs) {
            BatchItem item;
            item.resource = catalog_.find(req.resourceId);

            if (!item.resource) {
                item.reason = "resource not found: " + req.resourceId;
                batchValid = false;
                items.push_back(std::move(item));
                continue;
            }

            if (req.quantity <= 0) {
                item.reason = "quantity must be positive";
                batchValid = false;
                items.push_back(std::move(item));
                continue;
            }

            try {
                item.preTaxCost = costFor(
                    *item.resource,
                    req.resourceId,
                    req.quantity,
                    &item.vendor);

                item.tax = taxFor(*item.resource, item.preTaxCost);
                item.postTaxCost = item.preTaxCost + item.tax;

                Budget* selectedBudget = nullptr;

                if (req.department.empty()) {
                    selectedBudget = &temporaryDefaultBudget;
                } else {
                    auto it = temporaryDepartmentBudgets.find(
                        req.department);

                    if (it == temporaryDepartmentBudgets.end()) {
                        throw std::invalid_argument(
                            "department not found: " + req.department);
                    }

                    selectedBudget = &it->second;
                }

                item.reason = selectedBudget->check(
                    item.resource->category(),
                    req.quantity,
                    item.postTaxCost,
                    item.resource->title());

                if (item.reason.empty()) {
                    selectedBudget->commit(
                        item.resource->category(),
                        req.quantity,
                        item.postTaxCost,
                        item.resource->title());
                }
            } catch (const std::invalid_argument& e) {
                item.reason = e.what();
            }

            if (!item.reason.empty()) {
                batchValid = false;
            }

            items.push_back(std::move(item));
        }

        if (!batchValid) {
            for (std::size_t i = 0; i < reqs.size(); ++i) {
                const auto& req = reqs[i];
                const auto& item = items[i];

                const std::string reason =
                    item.reason.empty()
                        ? "batch rejected: all-or-nothing validation failed"
                        : item.reason;

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
                        req.department,
                        item.vendor));
            }

            return results;
        }

        // All requests passed preflight. Now commit real purchases.
        for (const auto& req : reqs) {
            results.push_back(
                purchase(req.department, req.resourceId, req.quantity));
        }

        return results;
    }

    // ==================================================
    // Independent request processing
    // ==================================================

    for (const auto& req : reqs) {
        const Resource* resource = catalog_.find(req.resourceId);

        Money preTaxCost;
        Money tax;
        Money postTaxCost;
        std::string why;
        std::string vendor;

        if (!resource) {
            why = "resource not found: " + req.resourceId;
        } else if (req.quantity <= 0) {
            why = "quantity must be positive";
        } else {
            try {
                preTaxCost = costFor(
                    *resource,
                    req.resourceId,
                    req.quantity,
                    &vendor);

                tax = taxFor(*resource, preTaxCost);
                postTaxCost = preTaxCost + tax;

                const Budget& selectedBudget = budgetFor(req.department);

                why = selectedBudget.check(
                    resource->category(),
                    req.quantity,
                    postTaxCost,
                    resource->title());
            } catch (const std::invalid_argument& e) {
                why = e.what();
            }
        }

        if (why.empty()) {
            results.push_back(
                purchase(req.department, req.resourceId, req.quantity));
        } else {
            results.push_back(
                record(
                    resource,
                    req.resourceId,
                    req.quantity,
                    preTaxCost,
                    tax,
                    postTaxCost,
                    false,
                    why,
                    false,
                    req.department,
                    vendor));
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
        if (!rec.approved) {
            continue;
        }

        const std::string cancellationMarker =
            "Cancellation of order #" + std::to_string(rec.orderNo);

        bool cancelled = false;

        for (const auto& other : history_) {
            if (other.cancelled && other.reason == cancellationMarker) {
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

void AcquisitionManager::printReport(std::ostream& os) const {
    os << "Order history (" << history_.size() << " records):\n";

    for (const auto& rec : history_) {
        os << "Order #" << rec.orderNo << " | ";

        if (!rec.department.empty()) {
            os << "Department: " << rec.department << " | ";
        }

        os << rec.title
           << " | quantity: " << rec.quantity;

        if (!rec.vendor.empty()) {
            os << " | Vendor: " << rec.vendor;
        }

        os << " | pre-tax: " << rec.preTaxCost
           << " | tax: " << rec.tax
           << " | post-tax: " << rec.postTaxCost
           << " | ";

        if (rec.cancelled) {
            os << "CANCELLED";
        } else if (rec.approved) {
            os << "APPROVED";
        } else {
            os << "REJECTED";
        }

        if (!rec.reason.empty()) {
            os << " | reason: " << rec.reason;
        }

        os << "\n";
    }

    os << "Total spent (post-tax): " << totalSpent() << "\n";
}

}  // namespace bookmgmt