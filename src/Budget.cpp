#include "bookmgmt/Budget.h"

#include <iomanip>
#include <ostream>
#include <stdexcept>

#include "bookmgmt/Exceptions.h"

namespace bookmgmt {

// Every category, in the order Budget::print() lists them.
const ResourceCategory kAllCategories[] = {
    ResourceCategory::Book,
    ResourceCategory::ElectronicResource,
    ResourceCategory::Journal,
    ResourceCategory::EBook,
    ResourceCategory::AudioBook,
    ResourceCategory::Thesis
};

Budget::Budget(Money total)
    : total_(total) {

    if (total_.isNegative()) {
        throw std::invalid_argument(
            "budget must not be negative"
        );
    }
}

// ------------------------------------------------------
// Q10: Year-end rollover
// ------------------------------------------------------
//
// Create a fresh Budget for the next year.
//
// Only a percentage of the CURRENTLY UNSPENT amount is
// carried forward.
//
// The current year's:
//   - spending,
//   - category usage,
//   - title usage
//
// are NOT copied.
//
// The configured category quotas ARE copied.
//
// Example:
//
//   Current budget = 10000
//   Spent          = 6000
//   Unspent        = 4000
//   Rollover       = 50%
//
//   Next budget = 4000 * 50% = 2000
// ------------------------------------------------------

Budget Budget::rollover(double percentage) const {

    if (percentage < 0.0 ||
        percentage > 100.0) {

        throw std::invalid_argument(
            "rollover percentage must be between 0 and 100"
        );
    }

    // Work with integer minor units so that the calculation
    // does not convert the monetary amount through double.
    //
    // Example:
    //
    //   remaining = 400000 paise
    //   percentage = 50
    //
    //   carry = 400000 * 50 / 100
    //
    // We use long double only for the percentage itself.
    const long double percentageValue =
        static_cast<long double>(percentage);

    const long double carryMinorUnits =
        static_cast<long double>(
            remaining().minorUnits()
        ) *
        percentageValue /
        100.0L;

    // A monetary amount must contain a whole number of
    // minor units. Round to the nearest minor unit.
    const auto roundedCarry =
        static_cast<std::int64_t>(
            carryMinorUnits + 0.5L
        );

    Budget nextYear(
        Money::fromMinor(roundedCarry)
    );

    // Quotas are configuration for the category.
    // They apply to the next year's budget as well.
    nextYear.quotas_ = quotas_;

    // Do NOT copy:
    //
    //   spent_
    //   usage_
    //   titleUsage_
    //
    // These represent this year's consumption and the new
    // year must start with fresh usage.
    return nextYear;
}

void Budget::setQuota(
    ResourceCategory c,
    Quota q) {

    if (q.maxUnits < 0 ||
        q.maxSpend.isNegative() ||
        q.maxTitles < -1) {

        throw std::invalid_argument(
            "quota limits must not be negative"
        );
    }

    quotas_[c] = q;
}

void Budget::removeQuota(
    ResourceCategory c) {

    quotas_.erase(c);
}

std::optional<Quota> Budget::quotaFor(
    ResourceCategory c) const {

    auto it = quotas_.find(c);

    if (it == quotas_.end()) {
        return std::nullopt;
    }

    return it->second;
}

Usage Budget::usageFor(
    ResourceCategory c) const {

    auto it = usage_.find(c);

    Usage result;

    if (it != usage_.end()) {
        result = it->second;
    }

    // Q7:
    // The number of different titles is the number
    // of entries in the title map for this category.
    auto titleIt = titleUsage_.find(c);

    if (titleIt != titleUsage_.end()) {
        result.differentTitles =
            static_cast<int>(
                titleIt->second.size()
            );
    }

    return result;
}

std::optional<int> Budget::unitsRemaining(
    ResourceCategory c) const {

    auto q = quotaFor(c);

    if (!q) {
        return std::nullopt;
    }

    return q->maxUnits - usageFor(c).units;
}

std::optional<Money> Budget::spendRemaining(
    ResourceCategory c) const {

    auto q = quotaFor(c);

    if (!q) {
        return std::nullopt;
    }

    return q->maxSpend - usageFor(c).spent;
}

Budget::Failure Budget::evaluate(
    ResourceCategory c,
    int units,
    Money cost,
    const std::string& title,
    std::string& why) const {

    // --------------------------------------------------
    // 1. Validate the number of units.
    // --------------------------------------------------

    if (units <= 0) {
        why = "quantity must be positive";
        return Failure::BadInput;
    }

    // --------------------------------------------------
    // 2. Validate the cost.
    // --------------------------------------------------

    if (cost.isNegative()) {
        why = "cost must not be negative";
        return Failure::BadInput;
    }

    // --------------------------------------------------
    // 3. Check the unit quota.
    // --------------------------------------------------

    if (auto left = unitsRemaining(c);
        left && units > *left) {

        why = std::string(categoryName(c)) +
              " unit quota exceeded: requested " +
              std::to_string(units) +
              ", " +
              std::to_string(*left) +
              " remaining";

        return Failure::Quota;
    }

    // --------------------------------------------------
    // 4. Q7: Check the different-title quota.
    // --------------------------------------------------

    if (auto q = quotaFor(c);
        q && q->maxTitles >= 0 && !title.empty()) {

        const auto categoryTitles =
            titleUsage_.find(c);

        bool titleAlreadyExists = false;

        if (categoryTitles != titleUsage_.end()) {
            titleAlreadyExists =
                categoryTitles->second.find(title)
                != categoryTitles->second.end();
        }

        const int currentTitles =
            usageFor(c).differentTitles;

        // Only a NEW title consumes a title slot.
        if (!titleAlreadyExists &&
            currentTitles >= q->maxTitles) {

            why = std::string(categoryName(c)) +
                  " title quota exceeded: maximum " +
                  std::to_string(q->maxTitles) +
                  " different titles allowed";

            return Failure::Quota;
        }
    }

    // --------------------------------------------------
    // 5. Check the category spending quota.
    // --------------------------------------------------

    if (auto left = spendRemaining(c);
        left && cost > *left) {

        why = std::string(categoryName(c)) +
              " spend quota exceeded: cost " +
              cost.toString() +
              ", " +
              left->toString() +
              " remaining";

        return Failure::Quota;
    }

    // --------------------------------------------------
    // 6. Check the overall budget.
    // --------------------------------------------------

    if (cost > remaining()) {

        why = "overall budget exceeded: cost " +
              cost.toString() +
              ", " +
              remaining().toString() +
              " remaining";

        return Failure::Overall;
    }

    why.clear();

    return Failure::None;
}

std::string Budget::check(
    ResourceCategory c,
    int units,
    Money cost) const {

    std::string why;

    evaluate(
        c,
        units,
        cost,
        "",
        why
    );

    return why;
}

std::string Budget::check(
    ResourceCategory c,
    int units,
    Money cost,
    const std::string& title) const {

    std::string why;

    evaluate(
        c,
        units,
        cost,
        title,
        why
    );

    return why;
}

void Budget::commit(
    ResourceCategory c,
    int units,
    Money cost) {

    commit(
        c,
        units,
        cost,
        ""
    );
}

void Budget::commit(
    ResourceCategory c,
    int units,
    Money cost,
    const std::string& title) {

    std::string why;

    switch (evaluate(
        c,
        units,
        cost,
        title,
        why)) {

        case Failure::None:
            break;

        case Failure::BadInput:
            throw std::invalid_argument(why);

        case Failure::Quota:
            throw QuotaExceededError(why);

        case Failure::Overall:
            throw BudgetExceededError(why);
    }

    // --------------------------------------------------
    // Update normal usage.
    // --------------------------------------------------

    Usage& u = usage_[c];

    u.units += units;
    u.spent += cost;

    // --------------------------------------------------
    // Q7:
    // Record the title.
    // --------------------------------------------------

    if (!title.empty()) {

        titleUsage_[c][title] += units;

        u.differentTitles =
            static_cast<int>(
                titleUsage_[c].size()
            );
    }

    // --------------------------------------------------
    // Update overall spending.
    // --------------------------------------------------

    spent_ += cost;
}

// ------------------------------------------------------
// Q8: Refund a previously committed purchase.
// ------------------------------------------------------

void Budget::refund(
    ResourceCategory c,
    int units,
    Money cost,
    const std::string& title) {

    // --------------------------------------------------
    // Validate refund arguments.
    // --------------------------------------------------

    if (units <= 0) {
        throw std::invalid_argument(
            "refund quantity must be positive"
        );
    }

    if (cost.isNegative()) {
        throw std::invalid_argument(
            "refund cost must not be negative"
        );
    }

    // --------------------------------------------------
    // Find the existing category usage.
    // --------------------------------------------------

    auto usageIt = usage_.find(c);

    if (usageIt == usage_.end() ||
        usageIt->second.units < units ||
        usageIt->second.spent < cost ||
        spent_ < cost) {

        throw std::invalid_argument(
            "refund exceeds current budget usage"
        );
    }

    // --------------------------------------------------
    // Reverse category units and spending.
    // --------------------------------------------------

    Usage& u = usageIt->second;

    u.units -= units;
    u.spent -= cost;

    // --------------------------------------------------
    // Q7:
    // Reverse title usage.
    // --------------------------------------------------

    if (!title.empty()) {

        auto categoryTitles =
            titleUsage_.find(c);

        if (categoryTitles == titleUsage_.end()) {
            throw std::invalid_argument(
                "title was not previously purchased"
            );
        }

        auto titleIt =
            categoryTitles->second.find(title);

        if (titleIt == categoryTitles->second.end() ||
            titleIt->second < units) {

            throw std::invalid_argument(
                "refund exceeds title usage"
            );
        }

        titleIt->second -= units;

        // If no units of this title remain,
        // remove the title completely.
        if (titleIt->second == 0) {
            categoryTitles->second.erase(titleIt);
        }

        // Remove an empty category title map.
        if (categoryTitles->second.empty()) {
            titleUsage_.erase(categoryTitles);
        }

        u.differentTitles =
            static_cast<int>(
                titleUsage_[c].size()
            );
    }

    // --------------------------------------------------
    // Reverse overall spending.
    // --------------------------------------------------

    spent_ -= cost;

    // --------------------------------------------------
    // Remove empty usage entry.
    // --------------------------------------------------

    if (u.units == 0 &&
        u.spent.isZero()) {

        usage_.erase(usageIt);
    }
}

void Budget::print(
    std::ostream& os) const {

    os << "Budget: total "
       << total_
       << ", spent "
       << spent_
       << ", remaining "
       << remaining()
       << "\n";

    os << std::left
       << std::setw(22)
       << "  Category"
       << std::setw(18)
       << "Units used/max"
       << std::setw(20)
       << "Titles used/max"
       << "Spend used/max\n";

    for (ResourceCategory c : kAllCategories) {

        const Usage u =
            usageFor(c);

        const auto q =
            quotaFor(c);

        // ----------------------------------------------
        // Units
        // ----------------------------------------------

        const std::string units =
            std::to_string(u.units) +
            "/" +
            (q
                ? std::to_string(q->maxUnits)
                : "-");

        // ----------------------------------------------
        // Different titles
        // ----------------------------------------------

        std::string titles =
            std::to_string(u.differentTitles) +
            "/";

        if (q) {

            if (q->maxTitles < 0) {

                titles += "-";

            } else {

                titles +=
                    std::to_string(
                        q->maxTitles
                    );
            }

        } else {

            titles += "-";
        }

        // ----------------------------------------------
        // Spending
        // ----------------------------------------------

        const std::string spend =
            u.spent.toString() +
            "/" +
            (q
                ? q->maxSpend.toString()
                : "-");

        os << "  "
           << std::setw(20)
           << categoryName(c)
           << std::setw(18)
           << units
           << std::setw(20)
           << titles
           << spend
           << "\n";
    }
}

}  // namespace bookmgmt
