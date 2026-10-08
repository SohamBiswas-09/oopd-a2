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

void Budget::setQuota(ResourceCategory c, Quota q) {

    if (q.maxUnits < 0 ||
        q.maxSpend.isNegative() ||
        q.maxTitles < -1) {

        throw std::invalid_argument(
            "quota limits must not be negative"
        );
    }

    quotas_[c] = q;
}

void Budget::removeQuota(ResourceCategory c) {
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
            static_cast<int>(titleIt->second.size());
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
    //
    // If the title already exists, its unit count is
    // increased but the number of different titles
    // remains unchanged.
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

void Budget::print(std::ostream& os) const {

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

        const Usage u = usageFor(c);
        const auto q = quotaFor(c);

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
                    std::to_string(q->maxTitles);
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
