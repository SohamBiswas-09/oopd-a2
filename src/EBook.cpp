#include "bookmgmt/EBook.h"

#include <ostream>
#include <stdexcept>
#include <utility>

namespace bookmgmt {

EBook::EBook(std::string id,
             std::string title,
             std::vector<std::string> authors,
             std::string isbn,
             std::string publisher,
             int year,
             Money pricePerSeat,
             std::string accessUrl,
             LicenseModel license,
             Money platformFee,
             std::string fileFormat,
             bool drmProtected)
    : ElectronicResource(std::move(id),
                         std::move(title),
                         std::move(publisher),
                         year,
                         pricePerSeat,
                         std::move(accessUrl),
                         license,
                         platformFee),
      authors_(std::move(authors)),
      isbn_(std::move(isbn)),
      fileFormat_(std::move(fileFormat)),
      drmProtected_(drmProtected) {

    if (authors_.empty()) {
        throw std::invalid_argument(
            "EBook must have at least one author"
        );
    }

    if (isbn_.empty()) {
        throw std::invalid_argument(
            "EBook ISBN must not be empty"
        );
    }

    if (fileFormat_ != "PDF" &&
        fileFormat_ != "EPUB" &&
        fileFormat_ != "HTML") {
        throw std::invalid_argument(
            "EBook file format must be PDF, EPUB or HTML"
        );
    }
}

void EBook::printDetails(std::ostream& os) const {
    ElectronicResource::printDetails(os);

    os << "  authors: ";

    for (std::size_t i = 0; i < authors_.size(); ++i) {
        if (i > 0) {
            os << ", ";
        }

        os << authors_[i];
    }

    os << "\n"
       << "  isbn: " << isbn_ << "\n"
       << "  file format: " << fileFormat_ << "\n"
       << "  drm protected: "
       << (drmProtected_ ? "yes" : "no")
       << "\n";
}

}  // namespace bookmgmt
