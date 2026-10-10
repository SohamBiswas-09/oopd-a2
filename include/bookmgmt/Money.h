#pragma once

#include <cstdint>
#include <iosfwd>
#include <string>

namespace bookmgmt {

class Money {
public:
    Money() = default;

    // Existing calls default to INR.
    static Money fromMinor(
        std::int64_t minor,
        const std::string& currency = "INR"
    ) {
        return Money(minor, currency);
    }

    static Money of(
        std::int64_t major,
        int minor = 0,
        const std::string& currency = "INR"
    );

    std::int64_t minorUnits() const {
        return minor_;
    }

    const std::string& currencyCode() const {
        return currency_;
    }

    double toDouble() const {
        return static_cast<double>(minor_) / 100.0;
    }

    std::string toString() const;

    bool isZero() const {
        return minor_ == 0;
    }

    bool isNegative() const {
        return minor_ < 0;
    }

    Money& operator+=(Money other);
    Money& operator-=(Money other);
    Money& operator*=(std::int64_t factor);

    friend Money operator+(Money a, Money b) {
        return a += b;
    }

    friend Money operator-(Money a, Money b) {
        return a -= b;
    }

    friend Money operator*(Money a, std::int64_t factor) {
        return a *= factor;
    }

    friend Money operator*(std::int64_t factor, Money a) {
        return a *= factor;
    }

    friend bool operator==(Money a, Money b);
    friend bool operator!=(Money a, Money b);
    friend bool operator<(Money a, Money b);
    friend bool operator<=(Money a, Money b);
    friend bool operator>(Money a, Money b);
    friend bool operator>=(Money a, Money b);

private:
    explicit Money(
        std::int64_t minor,
        std::string currency = "INR"
    );

    static void requireSameCurrency(Money a, Money b);

    std::int64_t minor_ = 0;
    std::string currency_ = "INR";
};

std::ostream& operator<<(std::ostream& os, Money m);

}  // namespace bookmgmt