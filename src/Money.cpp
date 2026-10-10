#include "bookmgmt/Money.h"

#include <ostream>
#include <stdexcept>
#include <utility>

namespace bookmgmt {

Money::Money(std::int64_t minor, std::string currency)
    : minor_(minor), currency_(std::move(currency)) {
    if (currency_.empty()) {
        throw std::invalid_argument("currency code cannot be empty");
    }
}

Money Money::of(
    std::int64_t major,
    int minor,
    const std::string& currency
) {
    if (minor < 0 || minor > 99) {
        throw std::invalid_argument("minor part must be in 0..99");
    }

    const std::int64_t sign = major < 0 ? -1 : 1;
    return Money(major * 100 + sign * minor, currency);
}

std::string Money::toString() const {
    const std::int64_t absMinor =
        minor_ < 0 ? -minor_ : minor_;

    std::string result = std::to_string(absMinor / 100);
    const auto fraction = absMinor % 100;

    result += '.';

    if (fraction < 10) {
        result += '0';
    }

    result += std::to_string(fraction);

    return minor_ < 0 ? "-" + result : result;
}

void Money::requireSameCurrency(Money a, Money b) {
    if (a.currency_ != b.currency_) {
        throw std::invalid_argument(
            "currency mismatch: " + a.currency_
            + " and " + b.currency_
        );
    }
}

Money& Money::operator+=(Money other) {
    requireSameCurrency(*this, other);
    minor_ += other.minor_;
    return *this;
}

Money& Money::operator-=(Money other) {
    requireSameCurrency(*this, other);
    minor_ -= other.minor_;
    return *this;
}

Money& Money::operator*=(std::int64_t factor) {
    minor_ *= factor;
    return *this;
}

bool operator==(Money a, Money b) {
    Money::requireSameCurrency(a, b);
    return a.minor_ == b.minor_;
}

bool operator!=(Money a, Money b) {
    Money::requireSameCurrency(a, b);
    return a.minor_ != b.minor_;
}

bool operator<(Money a, Money b) {
    Money::requireSameCurrency(a, b);
    return a.minor_ < b.minor_;
}

bool operator<=(Money a, Money b) {
    Money::requireSameCurrency(a, b);
    return a.minor_ <= b.minor_;
}

bool operator>(Money a, Money b) {
    Money::requireSameCurrency(a, b);
    return a.minor_ > b.minor_;
}

bool operator>=(Money a, Money b) {
    Money::requireSameCurrency(a, b);
    return a.minor_ >= b.minor_;
}

std::ostream& operator<<(std::ostream& os, Money m) {
    return os << m.toString();
}

}  // namespace bookmgmt