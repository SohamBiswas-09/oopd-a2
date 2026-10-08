#pragma once

#include <iosfwd>
#include <string>

#include "bookmgmt/ElectronicResource.h"

namespace bookmgmt {

// An AudioBook is an ElectronicResource because it is a digital resource
// accessed through an electronic platform and therefore can reuse the
// existing access URL, license model, platform fee, digital-resource
// behavior, and pricing logic.
//
// Q3-specific data:
//   - narrator
//   - duration in minutes
class AudioBook : public ElectronicResource {
public:
    AudioBook(std::string id,
              std::string title,
              std::string narrator,
              int durationMinutes,
              std::string publisher,
              int year,
              Money pricePerSeat,
              std::string accessUrl,
              LicenseModel license = LicenseModel::AnnualSubscription,
              Money platformFee = Money{});

    const std::string& narrator() const {
        return narrator_;
    }

    int durationMinutes() const {
        return durationMinutes_;
    }

    ResourceCategory category() const override {
        return ResourceCategory::AudioBook;
    }

protected:
    void printDetails(std::ostream& os) const override;

private:
    std::string narrator_;
    int durationMinutes_;
};

}  // namespace bookmgmt
