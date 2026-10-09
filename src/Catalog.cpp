#include "bookmgmt/Catalog.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

#include "bookmgmt/Exceptions.h"
#include "bookmgmt/Book.h"
#include "bookmgmt/EBook.h"
#include "bookmgmt/Journal.h"

namespace bookmgmt {

namespace {

// Convert a string to lowercase for case-insensitive searches.
std::string lower(std::string s) {
    std::transform(
        s.begin(),
        s.end(),
        s.begin(),
        [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        }
    );

    return s;
}

}  // namespace

Resource& Catalog::add(std::unique_ptr<Resource> r) {
    if (!r) {
        throw std::invalid_argument(
            "cannot add a null resource"
        );
    }

    const std::string id = r->id();

    if (items_.count(id)) {
        throw DuplicateIdError(id);
    }

    Entry& e = items_[id];
    e.resource = std::move(r);

    return *e.resource;
}

bool Catalog::contains(const std::string& id) const {
    return items_.count(id) > 0;
}

Resource* Catalog::find(const std::string& id) {
    auto it = items_.find(id);

    return it == items_.end()
        ? nullptr
        : it->second.resource.get();
}

const Resource* Catalog::find(const std::string& id) const {
    auto it = items_.find(id);

    return it == items_.end()
        ? nullptr
        : it->second.resource.get();
}

Resource& Catalog::get(const std::string& id) {
    if (Resource* r = find(id)) {
        return *r;
    }

    throw NotFoundError(id);
}

const Resource& Catalog::get(const std::string& id) const {
    if (const Resource* r = find(id)) {
        return *r;
    }

    throw NotFoundError(id);
}

void Catalog::remove(const std::string& id) {
    if (items_.erase(id) == 0) {
        throw NotFoundError(id);
    }
}

int Catalog::holdings(const std::string& id) const {
    auto it = items_.find(id);

    if (it == items_.end()) {
        throw NotFoundError(id);
    }

    return it->second.holdings;
}

void Catalog::addHoldings(
    const std::string& id,
    int units
) {
    auto it = items_.find(id);

    if (it == items_.end()) {
        throw NotFoundError(id);
    }

    if (it->second.holdings + units < 0) {
        throw std::invalid_argument(
            "holdings cannot become negative"
        );
    }

    it->second.holdings += units;
}

std::vector<const Resource*> Catalog::where(
    const std::function<bool(const Resource&)>& pred
) const {
    std::vector<const Resource*> out;

    // items_ is a std::map, so results are in resource-ID order.
    for (const auto& [id, entry] : items_) {
        (void)id;

        if (pred(*entry.resource)) {
            out.push_back(entry.resource.get());
        }
    }

    return out;
}

std::vector<const Resource*> Catalog::all() const {
    return where([](const Resource&) {
        return true;
    });
}

std::vector<const Resource*> Catalog::byCategory(
    ResourceCategory c
) const {
    return where([c](const Resource& r) {
        return r.category() == c;
    });
}

std::vector<const Resource*> Catalog::searchTitle(
    const std::string& text
) const {
    const std::string needle = lower(text);

    return where([&needle](const Resource& r) {
        return lower(r.title()).find(needle)
            != std::string::npos;
    });
}

// ============================================================
// Q13: Search by author
// ============================================================

std::vector<const Resource*> Catalog::searchAuthor(
    const std::string& text
) const {
    const std::string needle = lower(text);

    // An empty query should not match every resource.
    if (needle.empty()) {
        return {};
    }

    return where([&needle](const Resource& resource) {
        const auto matches = [&needle](
            const std::vector<std::string>& authors
        ) {
            for (const std::string& author : authors) {
                if (lower(author).find(needle)
                    != std::string::npos) {
                    return true;
                }
            }

            return false;
        };

        // Print books.
        if (const auto* book =
                dynamic_cast<const Book*>(&resource)) {
            return matches(book->authors());
        }

        // E-books.
        if (const auto* ebook =
                dynamic_cast<const EBook*>(&resource)) {
            return matches(ebook->authors());
        }

        return false;
    });
}

// ============================================================
// Q13: Search by ISBN or ISSN
// ============================================================

std::vector<const Resource*> Catalog::searchIsbnIssn(
    const std::string& text
) const {
    const std::string needle = lower(text);

    if (needle.empty()) {
        return {};
    }

    return where([&needle](const Resource& resource) {
        // ISBN for print books.
        if (const auto* book =
                dynamic_cast<const Book*>(&resource)) {
            return lower(book->isbn()).find(needle)
                != std::string::npos;
        }

        // ISBN for e-books.
        if (const auto* ebook =
                dynamic_cast<const EBook*>(&resource)) {
            return lower(ebook->isbn()).find(needle)
                != std::string::npos;
        }

        // ISSN for journals.
        if (const auto* journal =
                dynamic_cast<const Journal*>(&resource)) {
            return lower(journal->issn()).find(needle)
                != std::string::npos;
        }

        return false;
    });
}

// ============================================================
// Q13: Search by publication-year range
// ============================================================

std::vector<const Resource*> Catalog::searchYearRange(
    int fromYear,
    int toYear
) const {
    if (fromYear > toYear) {
        throw std::invalid_argument(
            "start year must not exceed end year"
        );
    }

    return where([fromYear, toYear](const Resource& resource) {
        const int year = resource.year();

        // Both boundary years are included.
        return year >= fromYear && year <= toYear;
    });
}

}  // namespace bookmgmt