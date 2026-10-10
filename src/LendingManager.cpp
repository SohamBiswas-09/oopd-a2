#include "bookmgmt/LendingManager.h"

#include <stdexcept>
#include <string>

#include "bookmgmt/ElectronicResource.h"
#include "bookmgmt/Exceptions.h"

namespace bookmgmt {

namespace {

void validatePatronId(const std::string& patronId) {
    if (patronId.empty()) {
        throw std::invalid_argument("patron ID cannot be empty");
    }
}

}  // namespace

LendingManager::LendingManager(Catalog& catalog)
    : catalog_(catalog) {}

void LendingManager::borrow(
    const std::string& resourceId,
    const std::string& patronId
) {
    validatePatronId(patronId);

    const Resource& resource = catalog_.get(resourceId);

    if (resource.isDigital()) {
        throw std::invalid_argument(
            "cannot borrow a digital resource as a print copy"
        );
    }

    if (availableCopies(resourceId) <= 0) {
        throw LendingCapacityError(
            "no print copies available for resource: " + resourceId
        );
    }

    ++loans_[{resourceId, patronId}];
}

void LendingManager::returnCopy(
    const std::string& resourceId,
    const std::string& patronId
) {
    validatePatronId(patronId);
    catalog_.get(resourceId);

    const PatronResource key{resourceId, patronId};
    auto it = loans_.find(key);

    if (it == loans_.end() || it->second <= 0) {
        throw LoanNotFoundError(
            "no active loan for patron " + patronId
            + " and resource " + resourceId
        );
    }

    if (--it->second == 0) {
        loans_.erase(it);
    }
}

void LendingManager::openSession(
    const std::string& resourceId,
    const std::string& patronId
) {
    validatePatronId(patronId);

    const Resource& resource = catalog_.get(resourceId);

    if (!resource.isDigital()) {
        throw std::invalid_argument(
            "cannot open an electronic session for a print resource"
        );
    }

    if (availableSeats(resourceId) <= 0) {
        throw LendingCapacityError(
            "no licensed seats available for resource: " + resourceId
        );
    }

    ++sessions_[{resourceId, patronId}];
}

void LendingManager::closeSession(
    const std::string& resourceId,
    const std::string& patronId
) {
    validatePatronId(patronId);
    catalog_.get(resourceId);

    const PatronResource key{resourceId, patronId};
    auto it = sessions_.find(key);

    if (it == sessions_.end() || it->second <= 0) {
        throw SessionNotFoundError(
            "no active session for patron " + patronId
            + " and resource " + resourceId
        );
    }

    if (--it->second == 0) {
        sessions_.erase(it);
    }
}

int LendingManager::activeLoans(
    const std::string& resourceId
) const {
    catalog_.get(resourceId);

    int total = 0;

    for (const auto& [key, count] : loans_) {
        if (key.first == resourceId) {
            total += count;
        }
    }

    return total;
}

int LendingManager::activeSessions(
    const std::string& resourceId
) const {
    catalog_.get(resourceId);

    int total = 0;

    for (const auto& [key, count] : sessions_) {
        if (key.first == resourceId) {
            total += count;
        }
    }

    return total;
}

int LendingManager::availableCopies(
    const std::string& resourceId
) const {
    const Resource& resource = catalog_.get(resourceId);

    if (resource.isDigital()) {
        throw std::invalid_argument(
            "print-copy availability requested for a digital resource"
        );
    }

    return catalog_.holdings(resourceId) - activeLoans(resourceId);
}

int LendingManager::availableSeats(
    const std::string& resourceId
) const {
    const Resource& resource = catalog_.get(resourceId);

    if (!resource.isDigital()) {
        throw std::invalid_argument(
            "seat availability requested for a print resource"
        );
    }

    return catalog_.holdings(resourceId) - activeSessions(resourceId);
}

}  // namespace bookmgmt
