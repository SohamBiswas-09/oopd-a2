#pragma once
// Budget: an overall spending limit plus optional per-category purchase quotas.
//
// A quota limits:
//   1. the number of units,
//   2. the amount of money spent,
//   3. the number of different titles.
//
// A category without a quota is limited only by the overall budget.

#include <iosfwd>
#include <map>
#include <optional>
#include <string>

#include "bookmgmt/Money.h"
#include "bookmgmt/Resource.h"

namespace bookmgmt {

struct Quota {
    int maxUnits;
    Money maxSpend;

    // Q7:
    // Maximum number of different titles allowed
    // in this category.
    //
    // -1 means there is no limit.
    int maxTitles = -1;
};

struct Usage {
    int units = 0;
    Money spent;

    // Q7:
    // Number of different titles currently purchased
    // in this category.
    int differentTitles = 0;
};

class Budget {
public:
    explicit Budget(Money total);

    Money total() const {
        return total_;
    }

    Money spent() const {
        return spent_;
    }

    Money remaining() const {
        return total_ - spent_;
    }

    void setQuota(ResourceCategory c, Quota q);
    void removeQuota(ResourceCategory c);

    std::optional<Quota> quotaFor(ResourceCategory c) const;

    Usage usageFor(ResourceCategory c) const;

    std::optional<int> unitsRemaining(ResourceCategory c) const;

    std::optional<Money> spendRemaining(ResourceCategory c) const;

    // Original API.
    //
    // This version does not know the title, so it only checks
    // units, spending and overall budget.
    std::string check(
        ResourceCategory c,
        int units,
        Money cost
    ) const;

    // Q7 version.
    //
    // The title is used to determine whether this purchase
    // introduces a new title to the category.
    std::string check(
        ResourceCategory c,
        int units,
        Money cost,
        const std::string& title
    ) const;

    // Original API.
    void commit(
        ResourceCategory c,
        int units,
        Money cost
    );

    // Q7 version.
    //
    // Records the title so that repeated purchases of the
    // same title do not consume another title slot.
    void commit(
        ResourceCategory c,
        int units,
        Money cost,
        const std::string& title
    );

    // Q8:
    // Reverses a previously committed purchase.
    //
    // This refunds:
    //   - category units,
    //   - category spending,
    //   - overall spending,
    //   - Q7 title usage.
    void refund(
        ResourceCategory c,
        int units,
        Money cost,
        const std::string& title
    );

    void print(std::ostream& os) const;

private:
    enum class Failure {
        None,
        BadInput,
        Quota,
        Overall
    };

    Failure evaluate(
        ResourceCategory c,
        int units,
        Money cost,
        const std::string& title,
        std::string& why
    ) const;

    Money total_;
    Money spent_;

    std::map<ResourceCategory, Quota> quotas_;
    std::map<ResourceCategory, Usage> usage_;

    // Q7:
    //
    // For each category, store every title that has been
    // purchased and the number of units purchased for it.
    //
    // Example:
    //
    // Book -> {
    //     "Clean Code": 5,
    //     "Design Patterns": 2
    // }
    //
    // The size of the inner map is the number of
    // different titles.
    std::map<
        ResourceCategory,
        std::map<std::string, int>
    > titleUsage_;
};

}  // namespace bookmgmt
