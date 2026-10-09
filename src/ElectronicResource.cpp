#include "bookmgmt/ElectronicResource.h"

#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace bookmgmt {

const char* licenseName(LicenseModel m) {
    switch (m) {
        case LicenseModel::Perpetual:
            return "Perpetual";

        case LicenseModel::AnnualSubscription:
            return "Annual subscription";
    }

    return "Unknown";
}

ElectronicResource::ElectronicResource(
    std::string id,
    std::string title,
    std::string publisher,
    int year,
    Money pricePerSeat,
    std::string accessUrl,
    LicenseModel license,
    Money platformFee)
    : Resource(std::move(id),
               std::move(title),
               std::move(publisher),
               year,
               pricePerSeat),
      accessUrl_(std::move(accessUrl)),
      license_(license),
      platformFee_(platformFee) {
    if (platformFee_.isNegative()) {
        throw std::invalid_argument(
            "platform fee must not be negative");
    }
}

Money ElectronicResource::costFor(int seats) const {
    requirePositive(seats);

    const int fullPriceSeats = seats < 50 ? seats : 50;
    const int discountedSeats = seats > 50 ? seats - 50 : 0;

    Money total = platformFee_
                + unitPrice() * fullPriceSeats;

    if (discountedSeats > 0) {
        const std::int64_t halfPrice =
            unitPrice().minorUnits() / 2;

        total += Money::fromMinor(halfPrice) * discountedSeats;
    }

    return total;
}

// Q12: Calculate the cost using a vendor-specific price per seat.
// The catalogue price remains unchanged.
Money ElectronicResource::costForAtPrice(
    int seats, Money vendorPrice) const {
    requirePositive(seats);

    if (vendorPrice.isNegative()) {
        throw std::invalid_argument(
            "vendor price must not be negative");
    }

    const int fullPriceSeats = seats < 50 ? seats : 50;
    const int discountedSeats = seats > 50 ? seats - 50 : 0;

    Money total = platformFee_
                + vendorPrice * fullPriceSeats;

    if (discountedSeats > 0) {
        const std::int64_t halfPrice =
            vendorPrice.minorUnits() / 2;

        total += Money::fromMinor(halfPrice) * discountedSeats;
    }

    return total;
}

void ElectronicResource::printDetails(std::ostream& os) const {
    os << "  access url: " << accessUrl_ << "\n"
       << "  license: " << licenseName(license_) << "\n"
       << "  platform fee: " << platformFee_ << "\n";
}

}  // namespace bookmgmt