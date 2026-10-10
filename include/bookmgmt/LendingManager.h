#pragma once

#include <map>
#include <string>
#include <utility>

#include "bookmgmt/Catalog.h"

namespace bookmgmt {

// Q14: Manages print-copy loans and electronic-resource sessions.
class LendingManager {
public:
    explicit LendingManager(Catalog& catalog);

    // Borrow one print copy for a patron.
    void borrow(const std::string& resourceId,
                const std::string& patronId);

    // Return a print copy previously borrowed by this patron.
    void returnCopy(const std::string& resourceId,
                    const std::string& patronId);

    // Open an electronic-resource session for a patron.
    void openSession(const std::string& resourceId,
                     const std::string& patronId);

    // Close an electronic-resource session previously opened by this patron.
    void closeSession(const std::string& resourceId,
                      const std::string& patronId);

    // Number of print copies currently on loan.
    int activeLoans(const std::string& resourceId) const;

    // Number of electronic sessions currently open.
    int activeSessions(const std::string& resourceId) const;

    // Number of print copies available for borrowing.
    int availableCopies(const std::string& resourceId) const;

    // Number of licensed electronic seats currently available.
    int availableSeats(const std::string& resourceId) const;

private:
    using PatronResource = std::pair<std::string, std::string>;

    Catalog& catalog_;

    // Maps (resource ID, patron ID) to the number of active loans/sessions.
    std::map<PatronResource, int> loans_;
    std::map<PatronResource, int> sessions_;
};

}  // namespace bookmgmt
