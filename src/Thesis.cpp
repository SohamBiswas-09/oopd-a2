#include "bookmgmt/Thesis.h"

#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace bookmgmt {

Thesis::Thesis(std::string id,
               std::string title,
               std::string university,
               std::string degree,
               std::string supervisor,
               std::string publisher,
               int year,
               Money unitPrice)
    : Resource(std::move(id),
               std::move(title),
               std::move(publisher),
               year,
               unitPrice),
      university_(std::move(university)),
      degree_(std::move(degree)),
      supervisor_(std::move(supervisor)) {
    if (university_.empty()) {
        throw std::invalid_argument("university must not be empty");
    }

    if (degree_.empty()) {
        throw std::invalid_argument("degree must not be empty");
    }

    if (supervisor_.empty()) {
        throw std::invalid_argument("supervisor must not be empty");
    }
}

Money Thesis::costFor(int copies) const {
    requirePositive(copies);

    Money total = unitPrice() * copies;

    if (copies >= 10) {
        const std::int64_t discountedMinorUnits =
            (total.minorUnits() * 90) / 100;

        return Money::fromMinor(discountedMinorUnits);
    }

    return total;
}

void Thesis::printDetails(std::ostream& os) const {
    os << "  university: " << university_ << "\n"
       << "  degree: " << degree_ << "\n"
       << "  supervisor: " << supervisor_ << "\n";
}

}  // namespace bookmgmt
