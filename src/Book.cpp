#include "bookmgmt/Book.h"

#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace bookmgmt {

std::string joinAuthors(const std::vector<std::string>& authors) {
    std::string out;

    for (std::size_t i = 0; i < authors.size(); ++i) {
        if (i > 0) {
            out += (i + 1 == authors.size()) ? " and " : ", ";
        }

        out += authors[i];
    }

    return out;
}

Book::Book(std::string id, std::string title, std::vector<std::string> authors,
           std::string isbn, std::string publisher, int year, Money unitPrice,
           int edition, Binding binding)
    : Resource(std::move(id),
               std::move(title),
               std::move(publisher),
               year,
               unitPrice),
      authors_(std::move(authors)),
      isbn_(std::move(isbn)),
      edition_(edition),
      binding_(binding) {
    if (edition_ < 1) {
        throw std::invalid_argument("edition must be >= 1");
    }
}

Money Book::costFor(int copies) const {
    requirePositive(copies);

    Money total;

    if (binding_ == Binding::Paperback) {
        total = unitPrice() * copies;
    } else {
        const std::int64_t listedPrice = unitPrice().minorUnits();

        const std::int64_t hardcoverPrice =
            listedPrice + (listedPrice * 20) / 100;

        total = Money::fromMinor(hardcoverPrice) * copies;
    }

    if (copies >= 10) {
        const std::int64_t discountedMinorUnits =
            (total.minorUnits() * 90) / 100;

        return Money::fromMinor(discountedMinorUnits);
    }

    return total;
}

// Q12: Calculate the cost using a vendor-specific price.
// The catalogue price remains unchanged.
Money Book::costForAtPrice(int copies, Money vendorPrice) const {
    requirePositive(copies);

    if (vendorPrice.isNegative()) {
        throw std::invalid_argument("vendor price must not be negative");
    }

    Money total;

    if (binding_ == Binding::Paperback) {
        total = vendorPrice * copies;
    } else {
        const std::int64_t listedPrice = vendorPrice.minorUnits();

        const std::int64_t hardcoverPrice =
            listedPrice + (listedPrice * 20) / 100;

        total = Money::fromMinor(hardcoverPrice) * copies;
    }

    if (copies >= 10) {
        const std::int64_t discountedMinorUnits =
            (total.minorUnits() * 90) / 100;

        return Money::fromMinor(discountedMinorUnits);
    }

    return total;
}

void Book::printDetails(std::ostream& os) const {
    os << "  authors: " << joinAuthors(authors_) << "\n"
       << "  isbn: " << isbn_ << "\n"
       << "  edition: " << edition_ << "\n"
       << "  binding: "
       << (binding_ == Binding::Hardcover ? "Hardcover" : "Paperback")
       << "\n";
}

}  // namespace bookmgmt