#pragma once

#include <string>

#include "bookmgmt/Resource.h"

namespace bookmgmt {

enum class LicenseModel { Perpetual, AnnualSubscription };

// An online resource licensed per concurrent-user seat (databases, e-journal
// packages, software, ...). unitPrice = price per seat.
// costFor(seats) = platformFee + unitPrice * seats.
class ElectronicResource : public Resource {
public:
    ElectronicResource(std::string id, std::string title, std::string publisher,
                       int year, Money pricePerSeat, std::string accessUrl,
                       LicenseModel license = LicenseModel::AnnualSubscription,
                       Money platformFee = Money{});

    const std::string& accessUrl() const { return accessUrl_; }
    LicenseModel license() const { return license_; }
    Money platformFee() const { return platformFee_; }

    ResourceCategory category() const override {
        return ResourceCategory::ElectronicResource;
    }

    bool isDigital() const override { return true; }

    Money costFor(int seats) const override;

    // Q12: Calculate the cost using a vendor-specific price per seat.
    // The platform fee and seat-pricing rules must remain unchanged.
    Money costForAtPrice(int seats, Money vendorPrice) const override;

protected:
    void printDetails(std::ostream& os) const override;

private:
    std::string accessUrl_;
    LicenseModel license_;
    Money platformFee_;
};

const char* licenseName(LicenseModel m);

}  // namespace bookmgmt
