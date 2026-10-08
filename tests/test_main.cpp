// Minimal self-contained test runner (no external framework needed).

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "bookmgmt/bookmgmt.h"

using namespace bookmgmt;

static void testJournal();
static void testEBook();

static int g_failures = 0;
static int g_checks = 0;

#define CHECK(cond)                                                              \
    do {                                                                         \
        ++g_checks;                                                              \
        if (!(cond)) {                                                           \
            ++g_failures;                                                        \
            std::cerr << __FILE__ << ":" << __LINE__                         \
                      << ": CHECK failed: " #cond << "\n";                      \
        }                                                                        \
    } while (0)

#define CHECK_THROWS(expr, ExType)            \
    do {                                      \
        bool thrown_ = false;                 \
        try {                                 \
            (void)(expr);                     \
        } catch (const ExType&) {             \
            thrown_ = true;                   \
        } catch (...) {                       \
        }                                     \
        CHECK(thrown_ && "expected " #ExType); \
    } while (0)


static void testMoney() {
    CHECK(Money::of(12, 5).toString() == "12.05");
    CHECK(Money::of(-3, 50).toString() == "-3.50");
    CHECK(Money::fromMinor(7).toString() == "0.07");
    CHECK(Money::of(10) + Money::of(0, 50) == Money::fromMinor(1050));
    CHECK(Money::of(3) * 4 == Money::of(12));
    CHECK(Money::of(1) < Money::of(2));
    CHECK_THROWS(Money::of(1, 100), std::invalid_argument);
}


static void testResourcesAndCost() {
    Book b("B1", "T", {"A", "B", "C"}, "isbn", "P",
           2020, Money::of(100));

    CHECK(b.category() == ResourceCategory::Book);
    CHECK(!b.isDigital());
    CHECK(b.costFor(3) == Money::of(300));
    CHECK_THROWS(b.costFor(0), std::invalid_argument);
    CHECK(joinAuthors(b.authors()) == "A, B and C");

    ElectronicResource e(
        "R1",
        "DB",
        "P",
        2026,
        Money::of(10),
        "url",
        LicenseModel::AnnualSubscription,
        Money::of(100)
    );

    CHECK(e.isDigital());
    CHECK(e.costFor(5) == Money::of(150));

    CHECK(e.category() == ResourceCategory::ElectronicResource);

    // Polymorphism through a base-class reference
    const Resource& r = e;

    CHECK(r.costFor(1) == Money::of(110));

    std::ostringstream os;
    os << r;

    CHECK(os.str().find("platform fee: 100.00") != std::string::npos);

    CHECK_THROWS(
        Book("", "T", {}, "", "", 2000, Money::of(1)),
        std::invalid_argument
    );

    CHECK_THROWS(
        Book("B", "T", {}, "", "", 2000, Money::fromMinor(-1)),
        std::invalid_argument
    );
}


static void testJournal() {
    Journal j(
        "J1",
        "ACM Computing Surveys",
        "1234-5678",
        12,
        "ACM",
        2026,
        Money::of(50)
    );

    CHECK(j.category() == ResourceCategory::Journal);
    CHECK(j.issn() == "1234-5678");
    CHECK(j.issuesPerYear() == 12);
    CHECK(j.subscriptionYears() == 1);
    CHECK(j.costFor(3) == Money::of(150));

    Journal j2(
        "J2",
        "Nature",
        "8765-4321",
        52,
        "Springer",
        2026,
        Money::of(100),
        3
    );

    CHECK(j2.subscriptionYears() == 3);
    CHECK(j2.costFor(2) == Money::of(600));

    CHECK_THROWS(
        Journal(
            "J3",
            "Invalid Journal",
            "0000-0000",
            12,
            "Publisher",
            2026,
            Money::of(50),
            0
        ),
        std::invalid_argument
    );

    std::stringstream os;

    j.print(os);

    CHECK(os.str().find("1234-5678") != std::string::npos);
    CHECK(os.str().find("issues per year: 12") != std::string::npos);
    CHECK(os.str().find("subscription years: 1") != std::string::npos);
}


/*
 * Q2: EBook
 *
 * An EBook is derived from ElectronicResource.
 *
 * The assignment requires:
 *   - authors
 *   - ISBN
 *   - file format
 *   - DRM-protected flag
 *   - inherited ElectronicResource pricing
 *   - separate ResourceCategory
 *   - printDetails() must call the parent version first
 *
 * Design note:
 * Book and EBook both contain book-related metadata such as authors
 * and ISBN. This creates duplication between the two classes.
 *
 * The duplication could be avoided by extracting the common book
 * metadata into a separate reusable class, such as BookMetadata,
 * and composing that object inside both Book and EBook.
 */
static void testEBook() {
    EBook e(
        "E1",
        "Clean Code",
        std::vector<std::string>{"Robert C. Martin"},
        "978-0132350884",
        "Prentice Hall",
        2008,
        Money::of(20),
        "https://example.com/clean-code",
        LicenseModel::AnnualSubscription,
        Money::of(100),
        "PDF",
        true
    );

    // EBook has its own category.
    CHECK(e.category() == ResourceCategory::EBook);

    // EBook-specific information.
    CHECK(e.authors().size() == 1);
    CHECK(e.authors()[0] == "Robert C. Martin");
    CHECK(e.isbn() == "978-0132350884");
    CHECK(e.fileFormat() == "PDF");
    CHECK(e.drmProtected());

    // EBook is still an ElectronicResource, so it is digital.
    CHECK(e.isDigital());

    // Pricing must be inherited unchanged from ElectronicResource.
    //
    // For an AnnualSubscription:
    //
    // platform fee + unit price * number of seats
    //
    // = 100 + 20 * 5
    // = 200
    CHECK(e.costFor(5) == Money::of(200));

    // Test polymorphism through the parent class.
    const ElectronicResource& er = e;
    CHECK(er.costFor(5) == Money::of(200));

    // printDetails() must include the inherited information
    // and then the EBook-specific information.
    std::ostringstream os;
    e.print(os);

    CHECK(os.str().find("authors: Robert C. Martin") != std::string::npos);
    CHECK(os.str().find("isbn: 978-0132350884") != std::string::npos);
    CHECK(os.str().find("file format: PDF") != std::string::npos);
    CHECK(os.str().find("drm protected: yes") != std::string::npos);

    // Test another EBook without DRM.
    EBook e2(
        "E2",
        "Design Patterns",
        std::vector<std::string>{"Erich Gamma", "Richard Helm"},
        "978-0201633610",
        "Addison-Wesley",
        1994,
        Money::of(15),
        "https://example.com/design-patterns",
        LicenseModel::Perpetual,
        Money::of(0),
        "EPUB",
        false
    );

    CHECK(e2.category() == ResourceCategory::EBook);
    CHECK(e2.authors().size() == 2);
    CHECK(e2.isbn() == "978-0201633610");
    CHECK(e2.fileFormat() == "EPUB");
    CHECK(!e2.drmProtected());

    // Perpetual electronic resource:
    // platform fee + unit price * seats
    // = 0 + 15 * 2
    // = 30
    CHECK(e2.costFor(2) == Money::of(30));
}


static void testCatalog() {
    Catalog c;

    c.emplace<Book>(
        "B1",
        "Clean Code",
        std::vector<std::string>{"M"},
        "i",
        "P",
        2008,
        Money::of(1)
    );

    c.emplace<Book>(
        "B2",
        "Clean Architecture",
        std::vector<std::string>{"M"},
        "i",
        "P",
        2017,
        Money::of(1)
    );

    c.emplace<ElectronicResource>(
        "R1",
        "ACM Digital Library",
        "ACM",
        2026,
        Money::of(1),
        "url"
    );

    CHECK(c.size() == 3);
    CHECK(c.contains("B1"));
    CHECK(c.find("nope") == nullptr);

    CHECK_THROWS(
        c.get("nope"),
        NotFoundError
    );

    CHECK_THROWS(
        c.emplace<Book>(
            "B1",
            "dup",
            std::vector<std::string>{},
            "",
            "",
            1,
            Money::of(1)
        ),
        DuplicateIdError
    );

    CHECK(c.searchTitle("clean").size() == 2);
    CHECK(c.byCategory(ResourceCategory::ElectronicResource).size() == 1);
    CHECK(c.where([](const Resource& r) {
        return r.isDigital();
    }).size() == 1);

    CHECK(c.holdings("B1") == 0);

    c.addHoldings("B1", 3);

    CHECK(c.holdings("B1") == 3);

    CHECK_THROWS(
        c.addHoldings("B1", -5),
        std::invalid_argument
    );

    c.remove("R1");

    CHECK(c.size() == 2);

    CHECK_THROWS(
        c.remove("R1"),
        NotFoundError
    );
}


static void testBudget() {
    Budget b(Money::of(1000));

    b.setQuota(
        ResourceCategory::Book,
        {5, Money::of(400)}
    );

    CHECK(
        b.check(
            ResourceCategory::Book,
            2,
            Money::of(200)
        ).empty()
    );

    CHECK(
        !b.check(
            ResourceCategory::Book,
            6,
            Money::of(10)
        ).empty()
    );   // units

    CHECK(
        !b.check(
            ResourceCategory::Book,
            1,
            Money::of(401)
        ).empty()
    );  // spend

    CHECK(
        !b.check(
            ResourceCategory::ElectronicResource,
            1,
            Money::of(1001)
        ).empty()
    );  // overall

    CHECK(
        b.check(
            ResourceCategory::ElectronicResource,
            1,
            Money::of(900)
        ).empty()
    );    // no quota

    b.commit(
        ResourceCategory::Book,
        4,
        Money::of(300)
    );

    CHECK(b.spent() == Money::of(300));
    CHECK(*b.unitsRemaining(ResourceCategory::Book) == 1);
    CHECK(*b.spendRemaining(ResourceCategory::Book) == Money::of(100));
    CHECK(!b.unitsRemaining(ResourceCategory::ElectronicResource).has_value());

    CHECK_THROWS(
        b.commit(
            ResourceCategory::Book,
            2,
            Money::of(10)
        ),
        QuotaExceededError
    );

    CHECK_THROWS(
        b.commit(
            ResourceCategory::ElectronicResource,
            1,
            Money::of(800)
        ),
        BudgetExceededError
    );

    CHECK_THROWS(
        b.commit(
            ResourceCategory::ElectronicResource,
            0,
            Money::of(1)
        ),
        std::invalid_argument
    );

    CHECK(b.spent() == Money::of(300));
}


static void testAcquisition() {
    Catalog c;

    c.emplace<Book>(
        "B1",
        "Book",
        std::vector<std::string>{"A"},
        "i",
        "P",
        2020,
        Money::of(100)
    );

    c.emplace<ElectronicResource>(
        "R1",
        "DB",
        "P",
        2026,
        Money::of(10),
        "url",
        LicenseModel::AnnualSubscription,
        Money::of(50)
    );

    Budget b(Money::of(500));

    b.setQuota(
        ResourceCategory::Book,
        {3, Money::of(1000)}
    );

    AcquisitionManager acq(c, b);

    CHECK(acq.quote("R1", 5) == Money::of(100));

    std::string why;

    CHECK(
        acq.canPurchase("B1", 3, &why)
        && why.empty()
    );

    CHECK(
        !acq.canPurchase("B1", 4, &why)
        && !why.empty()
    );

    CHECK(
        !acq.canPurchase("nope", 1, &why)
    );

    const auto& rec = acq.purchase("B1", 2);

    CHECK(
        rec.approved
        && rec.cost == Money::of(200)
        && rec.orderNo == 1
    );

    CHECK(c.holdings("B1") == 2);

    CHECK_THROWS(
        acq.purchase("B1", 2),
        QuotaExceededError
    );

    CHECK_THROWS(
        acq.purchase("nope", 1),
        NotFoundError
    );

    CHECK(acq.history().size() == 1);

    auto res = acq.processBatch({
        {"R1", 10},
        {"R1", 100},
        {"B1", 1},
        {"zzz", 1},
        {"B1", 0}
    });

    CHECK(res.size() == 5);
    CHECK(res[0].approved && res[0].cost == Money::of(150));
    CHECK(!res[1].approved);
    CHECK(res[2].approved);
    CHECK(
        !res[3].approved
        && res[3].reason.find("not found") != std::string::npos
    );
    CHECK(!res[4].approved);

    CHECK(acq.totalSpent() == Money::of(450));
    CHECK(b.spent() == acq.totalSpent());
    CHECK(c.holdings("R1") == 10 && c.holdings("B1") == 3);
    CHECK(acq.history().size() == 6);
}


int main() {
    testMoney();
    testResourcesAndCost();
    testJournal();
    testEBook();
    testCatalog();
    testBudget();
    testAcquisition();

    std::cout
        << (g_checks - g_failures)
        << "/"
        << g_checks
        << " checks passed\n";

    return g_failures == 0 ? 0 : 1;
}
