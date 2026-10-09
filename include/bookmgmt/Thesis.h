#pragma once

#include <string>

#include "bookmgmt/Resource.h"

namespace bookmgmt {

class Thesis : public Resource {
public:
    Thesis(std::string id,
           std::string title,
           std::string university,
           std::string degree,
           std::string supervisor,
           std::string publisher,
           int year,
           Money unitPrice = Money{});

    const std::string& university() const {
        return university_;
    }

    const std::string& degree() const {
        return degree_;
    }

    const std::string& supervisor() const {
        return supervisor_;
    }

    ResourceCategory category() const override {
        return ResourceCategory::Thesis;
    }

    Money costFor(int copies) const override;

    // Q12: Calculate the cost using a vendor-specific price
    // while preserving the existing bulk discount.
    Money costForAtPrice(int copies, Money vendorPrice) const override;

protected:
    void printDetails(std::ostream& os) const override;

private:
    std::string university_;
    std::string degree_;
    std::string supervisor_;
};

}  // namespace bookmgmt