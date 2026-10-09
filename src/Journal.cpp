#include "bookmgmt/Journal.h"

#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace bookmgmt {

Journal::Journal(std::string id, std::string title, std::string issn,
                 int issuesPerYear, std::string publisher, int year,
                 Money unitPrice, int subscriptionYears)
    : Resource(std::move(id), std::move(title), std::move(publisher),
               year, unitPrice),
      issn_(std::move(issn)),
      issuesPerYear_(issuesPerYear),
      subscriptionYears_(subscriptionYears) {
    if (subscriptionYears_ < 1) {
        throw std::invalid_argument(
            "subscription years must be >= 1");
    }
}

Money Journal::costFor(int copies) const {
    requirePositive(copies);

    Money total = unitPrice() * copies * subscriptionYears_;

    if (copies >= 10) {
        const std::int64_t discountedMinorUnits =
            (total.minorUnits() * 90) / 100;

        return Money::fromMinor(discountedMinorUnits);
    }

    return total;
}

// Q12: Calculate the cost using a vendor-specific price.
// The catalogue price remains unchanged.
Money Journal::costForAtPrice(int copies, Money vendorPrice) const {
    requirePositive(copies);

    if (vendorPrice.isNegative()) {
        throw std::invalid_argument(
            "vendor price must not be negative");
    }

    Money total = vendorPrice * copies * subscriptionYears_;

    if (copies >= 10) {
        const std::int64_t discountedMinorUnits =
            (total.minorUnits() * 90) / 100;

        return Money::fromMinor(discountedMinorUnits);
    }

    return total;
}

void Journal::printDetails(std::ostream& os) const {
    os << "  issn: " << issn_ << "\n"
       << "  issues per year: " << issuesPerYear_ << "\n"
       << "  subscription years: " << subscriptionYears_ << "\n";
}

} // namespace bookmgmt