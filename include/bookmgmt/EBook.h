#pragma once

#include <string>
#include <vector>

#include "bookmgmt/ElectronicResource.h"

namespace bookmgmt {

// An EBook is an ElectronicResource with book-specific metadata.
// Q2 requires authors, ISBN, file format, and DRM status.
//
// Design note for the assignment:
// EBook is also conceptually a Book, so author/ISBN-related code is
// duplicated between Book and EBook. A possible way to avoid this is
// to extract the common book metadata into a separate reusable class
// or composition component and use it from both Book and EBook.
class EBook : public ElectronicResource {
public:
    EBook(std::string id,
          std::string title,
          std::vector<std::string> authors,
          std::string isbn,
          std::string publisher,
          int year,
          Money pricePerSeat,
          std::string accessUrl,
          LicenseModel license = LicenseModel::AnnualSubscription,
          Money platformFee = Money{},
          std::string fileFormat = "PDF",
          bool drmProtected = false);

    const std::vector<std::string>& authors() const { return authors_; }
    const std::string& isbn() const { return isbn_; }
    const std::string& fileFormat() const { return fileFormat_; }
    bool drmProtected() const { return drmProtected_; }

    ResourceCategory category() const override {
        return ResourceCategory::EBook;
    }

protected:
    void printDetails(std::ostream& os) const override;

private:
    std::vector<std::string> authors_;
    std::string isbn_;
    std::string fileFormat_;
    bool drmProtected_;
};

}  // namespace bookmgmt
