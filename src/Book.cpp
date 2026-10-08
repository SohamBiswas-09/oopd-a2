#include "bookmgmt/Book.h"

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

    // Paperback books use the normal listed price.
    if (binding_ == Binding::Paperback) {
        return unitPrice() * copies;
    }

    // Hardcover books cost 20% more.
    //
    // Money stores the amount in minor units (paise/cents),
    // so we perform the percentage calculation using integers
    // instead of floating-point arithmetic.
    const std::int64_t listedPrice = unitPrice().minorUnits();

    const std::int64_t hardcoverPrice =
        listedPrice + (listedPrice * 20) / 100;

    return Money::fromMinor(hardcoverPrice) * copies;
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
