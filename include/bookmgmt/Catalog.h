#pragma once

// Catalog: owns every Resource, keyed by its id, and tracks copies/seats held.

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "bookmgmt/Resource.h"

namespace bookmgmt {

class Catalog {
public:
    // Takes ownership. Throws DuplicateIdError if the id is already present.
    Resource& add(std::unique_ptr<Resource> r);

    // Convenience: catalog.emplace<Book>(ctor args...)
    template <typename T, typename... Args>
    T& emplace(Args&&... args) {
        auto p = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *p;
        add(std::move(p));
        return ref;
    }

    bool contains(const std::string& id) const;
    Resource* find(const std::string& id);
    const Resource* find(const std::string& id) const;

    Resource& get(const std::string& id);
    const Resource& get(const std::string& id) const;

    void remove(const std::string& id);

    std::size_t size() const {
        return items_.size();
    }

    bool empty() const {
        return items_.empty();
    }

    // Holdings: number of copies (print) or seats (electronic) owned.
    int holdings(const std::string& id) const;
    void addHoldings(const std::string& id, int units);

    // Existing catalogue queries.
    std::vector<const Resource*> all() const;
    std::vector<const Resource*> byCategory(ResourceCategory c) const;
    std::vector<const Resource*> searchTitle(
        const std::string& text
    ) const;

    // Q13: search resources by author.
    // Supports Book and EBook, with case-insensitive partial matching.
    std::vector<const Resource*> searchAuthor(
        const std::string& text
    ) const;

    // Q13: search ISBN for books/e-books and ISSN for journals.
    // Matching is case-insensitive and allows partial identifiers.
    std::vector<const Resource*> searchIsbnIssn(
        const std::string& text
    ) const;

    // Q13: search an inclusive publication-year range.
    // Throws std::invalid_argument if fromYear > toYear.
    std::vector<const Resource*> searchYearRange(
        int fromYear,
        int toYear
    ) const;

    std::vector<const Resource*> where(
        const std::function<bool(const Resource&)>& pred
    ) const;

private:
    struct Entry {
        std::unique_ptr<Resource> resource;
        int holdings = 0;
    };

    std::map<std::string, Entry> items_;
};

}  // namespace bookmgmt