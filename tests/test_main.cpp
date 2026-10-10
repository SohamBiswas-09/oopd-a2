// Minimal self-contained test runner.
// Q1-Q8 tests for the BookManagement assignment.

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "bookmgmt/bookmgmt.h"

using namespace bookmgmt;

// ============================================================
// Test framework
// ============================================================

static int g_failures = 0;
static int g_checks = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        ++g_checks;                                                        \
        if (!(cond)) {                                                     \
            ++g_failures;                                                  \
            std::cerr << __FILE__ << ":" << __LINE__                       \
                      << ": CHECK failed: " #cond << "\n";                 \
        }                                                                  \
    } while (0)

#define CHECK_THROWS(expr, ExType)                                         \
    do {                                                                   \
        bool thrown_ = false;                                              \
        try {                                                              \
            (void)(expr);                                                  \
        } catch (const ExType&) {                                          \
            thrown_ = true;                                                \
        } catch (...) {                                                    \
        }                                                                  \
        CHECK(thrown_ && "expected " #ExType);                             \
    } while (0)

// ============================================================
// Q1 - Money and basic resources
// ============================================================

static void testMoney() {

    CHECK(
        Money::of(12, 5).toString() == "12.05"
    );

    CHECK(
        Money::of(-3, 50).toString() == "-3.50"
    );

    CHECK(
        Money::fromMinor(7).toString() == "0.07"
    );

    CHECK(
        Money::of(10) + Money::of(0, 50)
        == Money::fromMinor(1050)
    );

    CHECK(
        Money::of(3) * 4
        == Money::of(12)
    );

    CHECK(
        4 * Money::of(3)
        == Money::of(12)
    );

    CHECK(
        Money::of(1) < Money::of(2)
    );

    CHECK(
        Money::of(2) > Money::of(1)
    );

    CHECK(
        Money::of(2) >= Money::of(2)
    );

    CHECK(
        Money::of(2) <= Money::of(2)
    );

    CHECK(
        Money::of(0).isZero()
    );

    CHECK(
        Money::of(-1).isNegative()
    );

    CHECK_THROWS(
        Money::of(1, 100),
        std::invalid_argument
    );
}

// ============================================================
// Basic Resource / Book / ElectronicResource tests
// ============================================================

static void testResourcesAndCost() {

    Book b(
        "B1",
        "Clean Code",
        std::vector<std::string>{
            "Robert C. Martin",
            "Author Two",
            "Author Three"
        },
        "ISBN1",
        "Prentice Hall",
        2008,
        Money::of(100)
    );

    CHECK(
        b.category() ==
        ResourceCategory::Book
    );

    CHECK(
        !b.isDigital()
    );

    CHECK(
        b.costFor(3) ==
        Money::of(300)
    );

    CHECK_THROWS(
        b.costFor(0),
        std::invalid_argument
    );

    CHECK(
        joinAuthors(b.authors())
        == "Robert C. Martin, Author Two and Author Three"
    );

    ElectronicResource e(
        "R1",
        "Digital Database",
        "Publisher",
        2026,
        Money::of(10),
        "https://example.com",
        LicenseModel::AnnualSubscription,
        Money::of(100)
    );

    CHECK(
        e.isDigital()
    );

    CHECK(
        e.category() ==
        ResourceCategory::ElectronicResource
    );

    CHECK(
        e.costFor(5) ==
        Money::of(150)
    );

    const Resource& r = e;

    CHECK(
        r.costFor(1) ==
        Money::of(110)
    );

    std::ostringstream os;

    os << r;

    CHECK(
        os.str().find("platform fee: 100.00")
        != std::string::npos
    );

    CHECK_THROWS(
        Book(
            "",
            "Invalid",
            {},
            "",
            "",
            2020,
            Money::of(1)
        ),
        std::invalid_argument
    );

    CHECK_THROWS(
        Book(
            "B",
            "Invalid",
            {},
            "",
            "",
            2020,
            Money::fromMinor(-1)
        ),
        std::invalid_argument
    );
}

// ============================================================
// Q1 - Journal
// ============================================================

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

    CHECK(
        j.category() ==
        ResourceCategory::Journal
    );

    CHECK(
        j.issn() ==
        "1234-5678"
    );

    CHECK(
        j.issuesPerYear() == 12
    );

    CHECK(
        j.subscriptionYears() == 1
    );

    CHECK(
        j.costFor(3) ==
        Money::of(150)
    );

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

    CHECK(
        j2.subscriptionYears() == 3
    );

    CHECK(
        j2.costFor(2) ==
        Money::of(600)
    );

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

    std::ostringstream os;

    // print() is public. printDetails() is protected.
    j.print(os);

    CHECK(
        os.str().find("1234-5678")
        != std::string::npos
    );

    CHECK(
        os.str().find("issues per year: 12")
        != std::string::npos
    );

    CHECK(
        os.str().find("subscription years: 1")
        != std::string::npos
    );
}

// ============================================================
// Q2 - EBook
// ============================================================

static void testEBook() {

    EBook e(
        "E1",
        "Clean Code",
        std::vector<std::string>{
            "Robert C. Martin"
        },
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

    CHECK(
        e.category() ==
        ResourceCategory::EBook
    );

    CHECK(
        e.authors().size() == 1
    );

    CHECK(
        e.authors()[0] ==
        "Robert C. Martin"
    );

    CHECK(
        e.isbn() ==
        "978-0132350884"
    );

    CHECK(
        e.fileFormat() ==
        "PDF"
    );

    CHECK(
        e.drmProtected()
    );

    CHECK(
        e.isDigital()
    );

    // Inherited ElectronicResource pricing:
    //
    // platform fee + price per seat * seats
    //
    // 100 + 20 * 5 = 200
    CHECK(
        e.costFor(5) ==
        Money::of(200)
    );

    const ElectronicResource& er = e;

    CHECK(
        er.costFor(5) ==
        Money::of(200)
    );

    std::ostringstream os;

    e.print(os);

    CHECK(
        os.str().find("authors: Robert C. Martin")
        != std::string::npos
    );

    CHECK(
        os.str().find("isbn: 978-0132350884")
        != std::string::npos
    );

    CHECK(
        os.str().find("file format: PDF")
        != std::string::npos
    );

    CHECK(
        os.str().find("drm protected: yes")
        != std::string::npos
    );

    EBook e2(
        "E2",
        "Design Patterns",
        std::vector<std::string>{
            "Erich Gamma",
            "Richard Helm"
        },
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

    CHECK(
        e2.category() ==
        ResourceCategory::EBook
    );

    CHECK(
        e2.authors().size() == 2
    );

    CHECK(
        e2.isbn() ==
        "978-0201633610"
    );

    CHECK(
        e2.fileFormat() ==
        "EPUB"
    );

    CHECK(
        !e2.drmProtected()
    );

    CHECK(
        e2.costFor(2) ==
        Money::of(30)
    );
}

// ============================================================
// Q3 - AudioBook and Thesis
// ============================================================

static void testAudioBookAndThesis() {

    // AudioBook is an ElectronicResource because it is a
    // digitally accessed resource.
    AudioBook audio(
        "A1",
        "Clean Architecture",
        "Robert C. Martin",
        720,
        "Publisher",
        2026,
        Money::of(50),
        "https://example.com/audio",
        LicenseModel::Perpetual,
        Money::of(0)
    );

    CHECK(
        audio.category() ==
        ResourceCategory::AudioBook
    );

    CHECK(
        audio.isDigital()
    );

    CHECK(
        audio.narrator() ==
        "Robert C. Martin"
    );

    CHECK(
        audio.durationMinutes() ==
        720
    );

    CHECK(
        audio.costFor(2) ==
        Money::of(100)
    );

    // Thesis is a print/resource item and is usually free.
    //
    // The extra publisher string belongs to the common Resource
    // information inherited by Thesis.
    Thesis thesis(
        "T1",
        "Machine Learning Research",
        "IIT Delhi",
        "M.Tech",
        "Prof. Example",
        "IIT Delhi",
        2026,
        Money::of(0)
    );

    CHECK(
        thesis.category() ==
        ResourceCategory::Thesis
    );

    CHECK(
        !thesis.isDigital()
    );

    CHECK(
        thesis.university() ==
        "IIT Delhi"
    );

    CHECK(
        thesis.degree() ==
        "M.Tech"
    );

    CHECK(
        thesis.supervisor() ==
        "Prof. Example"
    );

    CHECK(
        thesis.costFor(1) ==
        Money::of(0)
    );

    CHECK(
        thesis.costFor(5) ==
        Money::of(0)
    );
}

// ============================================================
// Q4 - Hardcover pricing
// ============================================================

static void testBookPricing() {

    Book paperback(
        "BP1",
        "Paperback Book",
        std::vector<std::string>{
            "Author"
        },
        "ISBN-P",
        "Publisher",
        2026,
        Money::of(100),
        1,
        Binding::Paperback
    );

    CHECK(
        paperback.binding() ==
        Binding::Paperback
    );

    CHECK(
        paperback.costFor(1) ==
        Money::of(100)
    );

    CHECK(
        paperback.costFor(3) ==
        Money::of(300)
    );

    Book hardcover(
        "BH1",
        "Hardcover Book",
        std::vector<std::string>{
            "Author"
        },
        "ISBN-H",
        "Publisher",
        2026,
        Money::of(100),
        1,
        Binding::Hardcover
    );

    CHECK(
        hardcover.binding() ==
        Binding::Hardcover
    );

    // 100 + 20% = 120
    CHECK(
        hardcover.costFor(1) ==
        Money::of(120)
    );

    CHECK(
        hardcover.costFor(3) ==
        Money::of(360)
    );

    CHECK_THROWS(
        hardcover.costFor(0),
        std::invalid_argument
    );
}

// ============================================================
// Q5 - Bulk discounts
// ============================================================

static void testBulkDiscounts() {

    Book paperback(
        "BP1",
        "Paperback Bulk",
        std::vector<std::string>{
            "Author"
        },
        "ISBN-P",
        "Publisher",
        2026,
        Money::of(100),
        1,
        Binding::Paperback
    );

    // Less than 10 copies: no discount.
    CHECK(
        paperback.costFor(9) ==
        Money::of(900)
    );

    // 10% discount at 10 copies.
    CHECK(
        paperback.costFor(10) ==
        Money::of(900)
    );

    // 20 * 100 = 2000, then 10% discount = 1800.
    CHECK(
        paperback.costFor(20) ==
        Money::of(1800)
    );

    Book hardcover(
        "BH1",
        "Hardcover Bulk",
        std::vector<std::string>{
            "Author"
        },
        "ISBN-H",
        "Publisher",
        2026,
        Money::of(100),
        1,
        Binding::Hardcover
    );

    // Hardcover unit price = 120.
    CHECK(
        hardcover.costFor(9) ==
        Money::of(1080)
    );

    // 10 * 120 = 1200, 10% discount = 1080.
    CHECK(
        hardcover.costFor(10) ==
        Money::of(1080)
    );

    // 20 * 120 = 2400, 10% discount = 2160.
    CHECK(
        hardcover.costFor(20) ==
        Money::of(2160)
    );

    ElectronicResource electronic(
        "R1",
        "Digital Resource",
        "Publisher",
        2026,
        Money::of(10),
        "url",
        LicenseModel::AnnualSubscription,
        Money::of(50)
    );

    // First 50 seats cost full price.
    CHECK(
        electronic.costFor(50) ==
        Money::of(550)
    );

    // Seat 51 costs half price.
    //
    // 50 + 50*10 + 1*5 = 555
    CHECK(
        electronic.costFor(51) ==
        Money::of(555)
    );

    // 60 seats:
    //
    // platform fee = 50
    // first 50 seats = 500
    // next 10 seats = 50
    // total = 600
    CHECK(
        electronic.costFor(60) ==
        Money::of(600)
    );

    Journal journal(
        "J1",
        "Journal",
        "ISSN1",
        12,
        "Publisher",
        2026,
        Money::of(50)
    );

    // 10 copies -> 10% discount.
    CHECK(
        journal.costFor(10) ==
        Money::of(450)
    );

    Thesis thesis(
        "T1",
        "Thesis",
        "IIT Delhi",
        "M.Tech",
        "Supervisor",
        "IIT Delhi",
        2026,
        Money::of(100)
    );

    CHECK(
        thesis.costFor(9) ==
        Money::of(900)
    );

    CHECK(
        thesis.costFor(10) ==
        Money::of(900)
    );
}

// ============================================================
// Catalog
// ============================================================

static void testCatalog() {

    Catalog c;

    c.emplace<Book>(
        "B1",
        "Clean Code",
        std::vector<std::string>{
            "Robert C. Martin"
        },
        "ISBN1",
        "Publisher",
        2008,
        Money::of(100)
    );

    c.emplace<Book>(
        "B2",
        "Clean Architecture",
        std::vector<std::string>{
            "Robert C. Martin"
        },
        "ISBN2",
        "Publisher",
        2017,
        Money::of(120)
    );

    c.emplace<ElectronicResource>(
        "R1",
        "ACM Digital Library",
        "ACM",
        2026,
        Money::of(10),
        "url"
    );

    CHECK(
        c.size() == 3
    );

    CHECK(
        c.contains("B1")
    );

    CHECK(
        !c.contains("B3")
    );

    CHECK(
        c.find("B3") == nullptr
    );

    CHECK(
        c.get("B1").title() ==
        "Clean Code"
    );

    CHECK_THROWS(
        c.get("B3"),
        NotFoundError
    );

    CHECK_THROWS(
        c.emplace<Book>(
            "B1",
            "Duplicate",
            std::vector<std::string>{},
            "ISBN3",
            "Publisher",
            2026,
            Money::of(10)
        ),
        DuplicateIdError
    );

    CHECK(
        c.searchTitle("clean").size() == 2
    );

    CHECK(
        c.byCategory(
            ResourceCategory::Book
        ).size() == 2
    );

    CHECK(
        c.byCategory(
            ResourceCategory::ElectronicResource
        ).size() == 1
    );

    CHECK(
        c.where([](const Resource& r) {
            return r.isDigital();
        }).size() == 1
    );

    CHECK(
        c.holdings("B1") == 0
    );

    c.addHoldings(
        "B1",
        3
    );

    CHECK(
        c.holdings("B1") == 3
    );

    CHECK_THROWS(
        c.addHoldings(
            "B1",
            -5
        ),
        std::invalid_argument
    );

    c.remove("R1");

    CHECK(
        c.size() == 2
    );

    CHECK_THROWS(
        c.remove("R1"),
        NotFoundError
    );
}

// ============================================================
// Q7 - Budget and different-title quota
// ============================================================

static void testBudget() {

    Budget b(
        Money::of(1000)
    );

    b.setQuota(
        ResourceCategory::Book,
        {
            10,
            Money::of(500),
            2
        }
    );

    CHECK(
        b.check(
            ResourceCategory::Book,
            2,
            Money::of(200),
            "Book A"
        ).empty()
    );

    b.commit(
        ResourceCategory::Book,
        2,
        Money::of(200),
        "Book A"
    );

    CHECK(
        b.spent() ==
        Money::of(200)
    );

    CHECK(
        b.remaining() ==
        Money::of(800)
    );

    Usage u =
        b.usageFor(
            ResourceCategory::Book
        );

    CHECK(
        u.units == 2
    );

    CHECK(
        u.spent ==
        Money::of(200)
    );

    CHECK(
        u.differentTitles == 1
    );

    // Buying the same title again does not create
    // another different-title entry.
    CHECK(
        b.check(
            ResourceCategory::Book,
            1,
            Money::of(100),
            "Book A"
        ).empty()
    );

    b.commit(
        ResourceCategory::Book,
        1,
        Money::of(100),
        "Book A"
    );

    CHECK(
        b.usageFor(
            ResourceCategory::Book
        ).differentTitles == 1
    );

    // Second different title is allowed.
    CHECK(
        b.check(
            ResourceCategory::Book,
            1,
            Money::of(100),
            "Book B"
        ).empty()
    );

    b.commit(
        ResourceCategory::Book,
        1,
        Money::of(100),
        "Book B"
    );

    CHECK(
        b.usageFor(
            ResourceCategory::Book
        ).differentTitles == 2
    );

    // Third different title should be rejected.
    CHECK(
        !b.check(
            ResourceCategory::Book,
            1,
            Money::of(50),
            "Book C"
        ).empty()
    );

    CHECK_THROWS(
        b.commit(
            ResourceCategory::Book,
            1,
            Money::of(50),
            "Book C"
        ),
        QuotaExceededError
    );

    CHECK(
        b.spent() ==
        Money::of(400)
    );

    CHECK(
        b.remaining() ==
        Money::of(600)
    );
}

// ============================================================
// Q6 - Acquisition and taxes
// ============================================================

static void testAcquisition() {

    Catalog c;

    c.emplace<Book>(
        "B1",
        "Book",
        std::vector<std::string>{
            "Author"
        },
        "ISBN",
        "Publisher",
        2020,
        Money::of(100)
    );

    c.emplace<ElectronicResource>(
        "R1",
        "Database",
        "Publisher",
        2026,
        Money::of(10),
        "url",
        LicenseModel::AnnualSubscription,
        Money::of(50)
    );

    Budget b(
        Money::of(1000)
    );

    b.setQuota(
        ResourceCategory::Book,
        {
            10,
            Money::of(1000),
            5
        }
    );

    AcquisitionManager acq(
        c,
        b
    );

    CHECK(
        acq.quote("B1", 2) ==
        Money::of(200)
    );

    std::string reason;

    CHECK(
        acq.canPurchase(
            "B1",
            2,
            &reason
        )
    );

    CHECK(
        reason.empty()
    );

    CHECK(
        !acq.canPurchase(
            "unknown",
            1,
            &reason
        )
    );

    CHECK(
        reason.find("not found")
        != std::string::npos
    );

    const PurchaseRecord& rec =
        acq.purchase(
            "B1",
            2
        );

    CHECK(
        rec.approved
    );

    CHECK(
        rec.orderNo == 1
    );

    CHECK(
        rec.resourceId == "B1"
    );

    CHECK(
        rec.quantity == 2
    );

    CHECK(
        rec.cost ==
        Money::of(200)
    );

    CHECK(
        c.holdings("B1") == 2
    );

    CHECK(
        b.spent() ==
        Money::of(200)
    );

    CHECK(
        acq.totalSpent() ==
        Money::of(200)
    );

    CHECK_THROWS(
        acq.purchase(
            "unknown",
            1
        ),
        NotFoundError
    );

    auto results =
        acq.processBatch(
            {
                {"R1", 10},
                {"R1", 100},
                {"B1", 1},
                {"unknown", 1},
                {"B1", 0}
            }
        );

    CHECK(
        results.size() == 5
    );

    CHECK(
        results[0].approved
    );

    CHECK(
        results[0].cost ==
        Money::of(150)
    );

    CHECK(
        !results[1].approved
    );

    CHECK(
        results[2].approved
    );

    CHECK(
        !results[3].approved
    );

    CHECK(
        results[3].reason.find("not found")
        != std::string::npos
    );

    CHECK(
        !results[4].approved
    );
}

// ============================================================
// Q6 - Tax configuration
// ============================================================

static void testTaxes() {

    Catalog c;

    c.emplace<Book>(
        "B1",
        "Taxed Book",
        std::vector<std::string>{
            "Author"
        },
        "ISBN-TAX",
        "Publisher",
        2026,
        Money::of(100)
    );

    c.emplace<EBook>(
        "E1",
        "Taxed EBook",
        std::vector<std::string>{
            "Author"
        },
        "ISBN-E",
        "Publisher",
        2026,
        Money::of(100),
        "url",
        LicenseModel::AnnualSubscription,
        Money::of(0),
        "PDF",
        false
    );

    Budget b(
        Money::of(1000)
    );

    b.setQuota(
        ResourceCategory::Book,
        {
            10,
            Money::of(1000),
            5
        }
    );

    b.setQuota(
        ResourceCategory::EBook,
        {
            100,
            Money::of(1000),
            5
        }
    );

    AcquisitionManager acq(
        c,
        b
    );

    acq.setPrintTaxRate(
        10.0
    );

    acq.setElectronicTaxRate(
        5.0
    );

    CHECK(
        acq.printTaxRate() ==
        10.0
    );

    CHECK(
        acq.electronicTaxRate() ==
        5.0
    );

    const PurchaseRecord& bookOrder =
        acq.purchase(
            "B1",
            2
        );

    CHECK(
        bookOrder.preTaxCost ==
        Money::of(200)
    );

    CHECK(
        bookOrder.tax ==
        Money::of(20)
    );

    CHECK(
        bookOrder.postTaxCost ==
        Money::of(220)
    );

    CHECK(
        bookOrder.cost ==
        Money::of(220)
    );

    const PurchaseRecord& ebookOrder =
        acq.purchase(
            "E1",
            2
        );

    CHECK(
        ebookOrder.preTaxCost ==
        Money::of(200)
    );

    CHECK(
        ebookOrder.tax ==
        Money::of(10)
    );

    CHECK(
        ebookOrder.postTaxCost ==
        Money::of(210)
    );

    CHECK(
        ebookOrder.cost ==
        Money::of(210)
    );

    CHECK(
        acq.totalSpent() ==
        Money::of(430)
    );

    CHECK(
        b.spent() ==
        Money::of(430)
    );
}

// ============================================================
// Q8 - Cancellation
// ============================================================

static void testCancellation() {

    Catalog c;

    c.emplace<Book>(
        "B1",
        "Clean Code",
        std::vector<std::string>{
            "Robert C. Martin"
        },
        "ISBN1",
        "Publisher",
        2008,
        Money::of(100)
    );

    c.emplace<Book>(
        "B2",
        "Design Patterns",
        std::vector<std::string>{
            "Erich Gamma"
        },
        "ISBN2",
        "Publisher",
        1994,
        Money::of(100)
    );

    Budget b(
        Money::of(1000)
    );

    b.setQuota(
        ResourceCategory::Book,
        {
            10,
            Money::of(1000),
            2
        }
    );

    AcquisitionManager acq(
        c,
        b
    );

    // --------------------------------------------------------
    // Purchase
    // --------------------------------------------------------

    const PurchaseRecord& order =
        acq.purchase(
            "B1",
            3
        );

    CHECK(
        order.approved
    );

    CHECK(
        !order.cancelled
    );

    CHECK(
        order.orderNo == 1
    );

    CHECK(
        order.resourceId == "B1"
    );

    CHECK(
        order.quantity == 3
    );

    CHECK(
        order.postTaxCost ==
        Money::of(300)
    );

    CHECK(
        c.holdings("B1") == 3
    );

    CHECK(
        b.spent() ==
        Money::of(300)
    );

    CHECK(
        b.usageFor(
            ResourceCategory::Book
        ).units == 3
    );

    CHECK(
        b.usageFor(
            ResourceCategory::Book
        ).spent ==
        Money::of(300)
    );

    CHECK(
        b.usageFor(
            ResourceCategory::Book
        ).differentTitles == 1
    );

    CHECK(
        acq.totalSpent() ==
        Money::of(300)
    );

    // --------------------------------------------------------
    // Cancel
    // --------------------------------------------------------

    const PurchaseRecord& cancellation =
        acq.cancel(
            order.orderNo
        );

    CHECK(
        !cancellation.approved
    );

    CHECK(
        cancellation.cancelled
    );

    CHECK(
        cancellation.resourceId ==
        "B1"
    );

    CHECK(
        cancellation.quantity == 3
    );

    CHECK(
        cancellation.reason ==
        "Cancellation of order #1"
    );

    // --------------------------------------------------------
    // History is preserved.
    // --------------------------------------------------------

    CHECK(
        acq.history().size() == 2
    );

    CHECK(
        acq.history()[0].orderNo == 1
    );

    CHECK(
        acq.history()[0].approved
    );

    CHECK(
        acq.history()[1].orderNo == 2
    );

    CHECK(
        !acq.history()[1].approved
    );

    // --------------------------------------------------------
    // Holdings are reduced.
    // --------------------------------------------------------

    CHECK(
        c.holdings("B1") == 0
    );

    // --------------------------------------------------------
    // Budget is refunded.
    // --------------------------------------------------------

    CHECK(
        b.spent() ==
        Money::of(0)
    );

    CHECK(
        b.remaining() ==
        Money::of(1000)
    );

    CHECK(
        b.usageFor(
            ResourceCategory::Book
        ).units == 0
    );

    CHECK(
        b.usageFor(
            ResourceCategory::Book
        ).spent ==
        Money::of(0)
    );

    CHECK(
        b.usageFor(
            ResourceCategory::Book
        ).differentTitles == 0
    );

    // Cancelled purchase should not contribute
    // to total spent.
    CHECK(
        acq.totalSpent() ==
        Money::of(0)
    );

    // --------------------------------------------------------
    // Q7 title quota is restored.
    // --------------------------------------------------------

    CHECK(
        acq.canPurchase(
            "B2",
            1
        )
    );

    const PurchaseRecord& secondOrder =
        acq.purchase(
            "B2",
            1
        );

    CHECK(
        secondOrder.approved
    );

    CHECK(
        b.usageFor(
            ResourceCategory::Book
        ).differentTitles == 1
    );

    CHECK(
        c.holdings("B2") == 1
    );

    // --------------------------------------------------------
    // Cannot cancel the same order twice.
    // --------------------------------------------------------

    CHECK_THROWS(
        acq.cancel(1),
        std::invalid_argument
    );

    // --------------------------------------------------------
    // Rejected orders cannot be cancelled.
    // --------------------------------------------------------

    std::string reason;

    CHECK(
        !acq.canPurchase(
            "B1",
            10,
            &reason
        )
    );

    const auto rejected =
        acq.processBatch(
            {
                {"B1", 10}
            }
        );

    CHECK(
        rejected.size() == 1
    );

    CHECK(
        !rejected[0].approved
    );

    CHECK_THROWS(
        acq.cancel(
            rejected[0].orderNo
        ),
        std::invalid_argument
    );

    // --------------------------------------------------------
    // Unknown order cannot be cancelled.
    // --------------------------------------------------------

    CHECK_THROWS(
        acq.cancel(9999),
        std::invalid_argument
    );
}

// ============================================================
// Q9: Department budgets
// ============================================================

static void testDepartments() {

    Catalog c;

    c.emplace<Book>(
        "B1",
        "Clean Code",
        std::vector<std::string>{"Robert C. Martin"},
        "ISBN1",
        "Publisher",
        2008,
        Money::of(100)
    );

    c.emplace<Book>(
        "B2",
        "Design Patterns",
        std::vector<std::string>{"Erich Gamma"},
        "ISBN2",
        "Publisher",
        1994,
        Money::of(200)
    );

    Budget defaultBudget(
        Money::of(10000)
    );

    AcquisitionManager acq(
        c,
        defaultBudget
    );

    // --------------------------------------------------------
    // Create two independent departments.
    // --------------------------------------------------------

    acq.addDepartment(
        "Computer Science",
        Money::of(1000)
    );

    acq.addDepartment(
        "Physics",
        Money::of(500)
    );

    CHECK(
        acq.hasDepartment(
            "Computer Science"
        )
    );

    CHECK(
        acq.hasDepartment(
            "Physics"
        )
    );

    CHECK(
        !acq.hasDepartment(
            "Mathematics"
        )
    );

    CHECK_THROWS(
        acq.addDepartment(
            "Computer Science",
            Money::of(500)
        ),
        std::invalid_argument
    );

    CHECK_THROWS(
        acq.addDepartment(
            "",
            Money::of(500)
        ),
        std::invalid_argument
    );

    CHECK_THROWS(
        acq.departmentBudget(
            "Mathematics"
        ),
        std::invalid_argument
    );

    // --------------------------------------------------------
    // Configure different quotas for each department.
    // --------------------------------------------------------

    acq.departmentBudget(
        "Computer Science"
    ).setQuota(
        ResourceCategory::Book,
        {
            10,
            Money::of(800),
            2
        }
    );

    acq.departmentBudget(
        "Physics"
    ).setQuota(
        ResourceCategory::Book,
        {
            2,
            Money::of(300),
            1
        }
    );

    // --------------------------------------------------------
    // Computer Science purchases one B1.
    // --------------------------------------------------------

    std::string reason;

    CHECK(
        acq.canPurchase(
            "Computer Science",
            "B1",
            1,
            &reason
        )
    );

    CHECK(
        reason.empty()
    );

    const PurchaseRecord& csOrder =
        acq.purchase(
            "Computer Science",
            "B1",
            1
        );

    CHECK(
        csOrder.approved
    );

    CHECK(
        csOrder.department
        == "Computer Science"
    );

    CHECK(
        csOrder.resourceId
        == "B1"
    );

    CHECK(
        csOrder.quantity
        == 1
    );

    CHECK(
        acq.departmentBudget(
            "Computer Science"
        ).spent()
        == Money::of(100)
    );

    // --------------------------------------------------------
    // Physics has not spent anything yet.
    // --------------------------------------------------------

    CHECK(
        acq.departmentBudget(
            "Physics"
        ).spent()
        == Money::of(0)
    );

    CHECK(
        acq.departmentBudget(
            "Physics"
        ).remaining()
        == Money::of(500)
    );

    // --------------------------------------------------------
    // Physics makes its own purchase.
    // --------------------------------------------------------

    const PurchaseRecord& physicsOrder =
        acq.purchase(
            "Physics",
            "B1",
            1
        );

    CHECK(
        physicsOrder.approved
    );

    CHECK(
        physicsOrder.department
        == "Physics"
    );

    CHECK(
        acq.departmentBudget(
            "Physics"
        ).spent()
        == Money::of(100)
    );

    // --------------------------------------------------------
    // The two budgets remain independent.
    // --------------------------------------------------------

    CHECK(
        acq.departmentBudget(
            "Computer Science"
        ).spent()
        == Money::of(100)
    );

    CHECK(
        acq.departmentBudget(
            "Physics"
        ).spent()
        == Money::of(100)
    );

    CHECK(
        defaultBudget.spent()
        == Money::of(0)
    );

    CHECK(
        c.holdings("B1")
        == 2
    );

    // --------------------------------------------------------
    // Physics already has one Book unit.
    // A second B1 is still allowed because the unit quota
    // is 2.
    // --------------------------------------------------------

    CHECK(
        acq.canPurchase(
            "Physics",
            "B1",
            1,
            &reason
        )
    );

    // --------------------------------------------------------
    // Two additional units would exceed Physics' unit quota.
    // --------------------------------------------------------

    CHECK(
        !acq.canPurchase(
            "Physics",
            "B1",
            2,
            &reason
        )
    );

    // --------------------------------------------------------
    // Computer Science has a larger budget and quota.
    // It can purchase two copies of B2.
    // --------------------------------------------------------

    CHECK(
        acq.canPurchase(
            "Computer Science",
            "B2",
            2,
            &reason
        )
    );

    const PurchaseRecord& csSecondOrder =
        acq.purchase(
            "Computer Science",
            "B2",
            2
        );

    CHECK(
        csSecondOrder.approved
    );

    CHECK(
        csSecondOrder.department
        == "Computer Science"
    );

    CHECK(
        acq.departmentBudget(
            "Computer Science"
        ).spent()
        == Money::of(500)
    );

    // --------------------------------------------------------
    // Physics remains unaffected by the Computer Science
    // purchase.
    // --------------------------------------------------------

    CHECK(
        acq.departmentBudget(
            "Physics"
        ).spent()
        == Money::of(100)
    );

    CHECK(
        acq.departmentBudget(
            "Physics"
        ).remaining()
        == Money::of(400)
    );

    // --------------------------------------------------------
    // Batch processing respects the department in each
    // PurchaseRequest.
    // --------------------------------------------------------

    const auto results =
        acq.processBatch(
            {
                {"B1", 1, "Computer Science"},
                {"B2", 1, "Physics"},
                {"B1", 1, "Mathematics"}
            }
        );

    CHECK(
        results.size()
        == 3
    );

    CHECK(
        results[0].approved
    );

    CHECK(
        results[0].department
        == "Computer Science"
    );

    CHECK(
        !results[1].approved
    );

    CHECK(
        results[1].department
        == "Physics"
    );

    CHECK(
        results[1].reason.find("titles")
        != std::string::npos
    );

    CHECK(
        !results[2].approved
    );

    CHECK(
        results[2].department
        == "Mathematics"
    );

    CHECK(
        results[2].reason.find("department")
        != std::string::npos
    );

    // --------------------------------------------------------
    // Cancellation refunds the budget of the department
    // that originally paid for the order.
    // --------------------------------------------------------

    const PurchaseRecord& cancellable =
        acq.purchase(
            "Computer Science",
            "B1",
            1
        );

    CHECK(
        cancellable.approved
    );

    CHECK(
        acq.departmentBudget(
            "Computer Science"
        ).spent()
        == Money::of(700)
    );

    const int orderNo =
        cancellable.orderNo;

    const PurchaseRecord& cancellation =
        acq.cancel(
            orderNo
        );

    CHECK(
        cancellation.cancelled
    );

    CHECK(
        cancellation.department
        == "Computer Science"
    );

    CHECK(
        acq.departmentBudget(
            "Computer Science"
        ).spent()
        == Money::of(600)
    );

    CHECK(
        acq.departmentBudget(
            "Physics"
        ).spent()
        == Money::of(100)
    );
}

// ============================================================
// Q10 - Year-end budget rollover
// ============================================================

static void testBudgetRollover() {

    // --------------------------------------------------------
    // Create this year's budget.
    //
    // Total budget = 1000
    // Spent        = 400
    // Unspent      = 600
    // --------------------------------------------------------

    Budget current(
        Money::of(1000)
    );

    // Configure a quota for the current year.
    current.setQuota(
        ResourceCategory::Book,
        {
            10,
            Money::of(500),
            2
        }
    );

    // Spend 400 from the budget.
    current.commit(
        ResourceCategory::Book,
        2,
        Money::of(400),
        "Clean Code"
    );

    CHECK(
        current.total() ==
        Money::of(1000)
    );

    CHECK(
        current.spent() ==
        Money::of(400)
    );

    CHECK(
        current.remaining() ==
        Money::of(600)
    );

    // --------------------------------------------------------
    // Q10: Carry forward 50% of the unspent amount.
    //
    // Unspent amount = 600
    // Rollover      = 50%
    //
    // 600 * 50 / 100 = 300
    //
    // Therefore next year's budget = 300.
    // --------------------------------------------------------

    Budget next =
        current.rollover(50.0);

    CHECK(
        next.total() ==
        Money::of(300)
    );

    CHECK(
        next.spent() ==
        Money::of(0)
    );

    CHECK(
        next.remaining() ==
        Money::of(300)
    );

    // --------------------------------------------------------
    // The quota configuration is carried forward.
    // --------------------------------------------------------

    const auto nextQuota =
        next.quotaFor(
            ResourceCategory::Book
        );

    CHECK(
        nextQuota.has_value()
    );

    CHECK(
        nextQuota->maxUnits == 10
    );

    CHECK(
        nextQuota->maxSpend ==
        Money::of(500)
    );

    CHECK(
        nextQuota->maxTitles == 2
    );

    // --------------------------------------------------------
    // Previous year's usage is NOT carried forward.
    //
    // The current year purchased:
    //   2 units
    //   ₹400
    //   1 different title
    //
    // Next year must start with zero usage.
    // --------------------------------------------------------

    const Usage nextUsage =
        next.usageFor(
            ResourceCategory::Book
        );

    CHECK(
        nextUsage.units == 0
    );

    CHECK(
        nextUsage.spent ==
        Money::of(0)
    );

    CHECK(
        nextUsage.differentTitles == 0
    );

    // --------------------------------------------------------
    // The original budget must remain unchanged.
    // --------------------------------------------------------

    CHECK(
        current.total() ==
        Money::of(1000)
    );

    CHECK(
        current.spent() ==
        Money::of(400)
    );

    CHECK(
        current.remaining() ==
        Money::of(600)
    );

    const Usage currentUsage =
        current.usageFor(
            ResourceCategory::Book
        );

    CHECK(
        currentUsage.units == 2
    );

    CHECK(
        currentUsage.spent ==
        Money::of(400)
    );

    CHECK(
        currentUsage.differentTitles == 1
    );

    // --------------------------------------------------------
    // 0% rollover.
    //
    // No part of the unspent amount is carried forward.
    // --------------------------------------------------------

    Budget zeroCarry =
        current.rollover(0.0);

    CHECK(
        zeroCarry.total() ==
        Money::of(0)
    );

    CHECK(
        zeroCarry.spent() ==
        Money::of(0)
    );

    CHECK(
        zeroCarry.remaining() ==
        Money::of(0)
    );

    // Quota configuration is still copied even though
    // the carried-forward budget is zero.
    CHECK(
        zeroCarry.quotaFor(
            ResourceCategory::Book
        ).has_value()
    );

    // --------------------------------------------------------
    // 100% rollover.
    //
    // Entire unspent amount is carried forward.
    //
    // Unspent = 600
    // 100% of 600 = 600
    // --------------------------------------------------------

    Budget fullCarry =
        current.rollover(100.0);

    CHECK(
        fullCarry.total() ==
        Money::of(600)
    );

    CHECK(
        fullCarry.spent() ==
        Money::of(0)
    );

    CHECK(
        fullCarry.remaining() ==
        Money::of(600)
    );

    // --------------------------------------------------------
    // Invalid percentages must be rejected.
    // --------------------------------------------------------

    CHECK_THROWS(
        current.rollover(-1.0),
        std::invalid_argument
    );

    CHECK_THROWS(
        current.rollover(101.0),
        std::invalid_argument
    );
}



// ============================================================
// Q11 - All-or-nothing batch processing
// ============================================================

static void testBatchAllOrNothing() {

    // --------------------------------------------------------
    // Create resources.
    // --------------------------------------------------------

    Catalog c;

    c.emplace<Book>(
        "B1",
        "Clean Code",
        std::vector<std::string>{
            "Robert C. Martin"
        },
        "ISBN1",
        "Publisher",
        2008,
        Money::of(100)
    );

    c.emplace<Book>(
        "B2",
        "Design Patterns",
        std::vector<std::string>{
            "Erich Gamma"
        },
        "ISBN2",
        "Publisher",
        1994,
        Money::of(200)
    );

    // --------------------------------------------------------
    // Create a budget with a book unit quota.
    //
    // The budget can buy at most 10 Book units and spend
    // at most ₹1000.
    // --------------------------------------------------------

    Budget b(
        Money::of(1000)
    );

    b.setQuota(
        ResourceCategory::Book,
        {
            10,
            Money::of(1000),
            5
        }
    );

    AcquisitionManager acq(
        c,
        b
    );

    // --------------------------------------------------------
    // Q11: A valid batch should commit completely.
    //
    // 2 copies of B1 = ₹200
    // 3 copies of B2 = ₹600
    //
    // Total = ₹800
    // --------------------------------------------------------

    const auto successful =
        acq.processBatch(
            {
                {"B1", 2},
                {"B2", 3}
            },
            true
        );

    CHECK(
        successful.size() == 2
    );

    CHECK(
        successful[0].approved
    );

    CHECK(
        successful[1].approved
    );

    CHECK(
        c.holdings("B1") == 2
    );

    CHECK(
        c.holdings("B2") == 3
    );

    CHECK(
        b.spent() ==
        Money::of(800)
    );

    CHECK(
        b.remaining() ==
        Money::of(200)
    );

    // --------------------------------------------------------
    // Q11: A failed all-or-nothing batch must not purchase
    // any request.
    //
    // Current usage:
    //   5 Book units
    //   ₹800 spent
    //
    // Batch:
    //   B1 x 1 -> would be valid
    //   B2 x 5 -> would make total units 11
    //
    // Therefore the complete batch must be rejected.
    // --------------------------------------------------------

    const int b1HoldingsBefore =
        c.holdings("B1");

    const int b2HoldingsBefore =
        c.holdings("B2");

    const Money spentBefore =
        b.spent();

    const Money remainingBefore =
        b.remaining();

    const auto failed =
        acq.processBatch(
            {
                {"B1", 1},
                {"unknown", 1}
            },
            true
        );

    CHECK(
        failed.size() == 2
    );

    // Both requests are rejected because the complete
    // all-or-nothing batch must fail.
    CHECK(
        !failed[0].approved
    );

    CHECK(
        !failed[1].approved
    );

    // The second request is rejected because the resource
    // does not exist.
    CHECK(
        failed[1].reason.find("not found")
        != std::string::npos
    );

    // --------------------------------------------------------
    // Most important Q11 checks:
    //
    // Nothing from the failed batch was purchased.
    // --------------------------------------------------------

    CHECK(
        c.holdings("B1") ==
        b1HoldingsBefore
    );

    CHECK(
        c.holdings("B2") ==
        b2HoldingsBefore
    );

    CHECK(
        b.spent() ==
        spentBefore
    );

    CHECK(
        b.remaining() ==
        remainingBefore
    );

    // --------------------------------------------------------
    // The rejected batch must not contribute to total spent.
    // --------------------------------------------------------

    CHECK(
        acq.totalSpent() ==
        Money::of(800)
    );

    // --------------------------------------------------------
    // Q11: allOrNothing = false must preserve the old
    // independent-processing behavior.
    //
    // At this point the budget has ₹200 remaining.
    //
    // B1 x 1 -> ₹100 -> approved
    // B2 x 1 -> ₹200 -> rejected
    //
    // The first request is still purchased even though the
    // second request fails.
    // --------------------------------------------------------

    const auto independent =
        acq.processBatch(
            {
                {"B1", 1},
                {"B2", 1}
            },
            false
        );

    CHECK(
        independent.size() == 2
    );

    CHECK(
        independent[0].approved
    );

    CHECK(
        !independent[1].approved
    );

    CHECK(
        c.holdings("B1") == 3
    );

    CHECK(
        c.holdings("B2") == 3
    );

    CHECK(
        b.spent() ==
        Money::of(900)
    );

    CHECK(
        b.remaining() ==
        Money::of(100)
    );

    // --------------------------------------------------------
    // The default argument must also preserve the old
    // behavior.
    //
    // Calling processBatch(requests) is equivalent to
    // processBatch(requests, false).
    // --------------------------------------------------------

    const auto defaultMode =
        acq.processBatch(
            {
                {"B1", 1},
                {"B2", 1}
            }
        );

    CHECK(
        defaultMode.size() == 2
    );

    CHECK(
        defaultMode[0].approved
    );

    CHECK(
        !defaultMode[1].approved
    );

    CHECK(
        c.holdings("B1") == 4
    );

    CHECK(
        c.holdings("B2") == 3
    );

    CHECK(
        b.spent() ==
        Money::of(1000)
    );

    CHECK(
        b.remaining() ==
        Money::of(0)
    );
}


// Helper used by Q12 vendor tests.
static Book& addBook(
    Catalog& catalog,
    const std::string& id,
    const std::string& title,
    Money price
) {
    return catalog.emplace<Book>(
        id,
        title,
        std::vector<std::string>{"Author"},
        "ISBN-" + id,
        "Publisher",
        2026,
        price
    );
}

// ============================================================
// Q12 - Vendor selection and vendor-aware purchases
// ============================================================

static void testCheapestVendorAndPurchase() {
    Catalog catalog;
    addBook(catalog, "B1", "Algorithms", Money::of(100));

    Budget budget(Money::of(1000));
    AcquisitionManager acq(catalog, budget);

    acq.addVendorOffer("B1", "Vendor A", Money::of(90));
    acq.addVendorOffer("B1", "Vendor B", Money::of(75));
    acq.addVendorOffer("B1", "Vendor C", Money::of(85));

    const VendorOffer cheapest = acq.cheapestVendor("B1");

    CHECK(cheapest.vendor == "Vendor B");
    CHECK(cheapest.unitPrice == Money::of(75));
    CHECK(acq.quote("B1", 2) == Money::of(150));

    const PurchaseRecord& order = acq.purchase("B1", 2);

    CHECK(order.approved);
    CHECK(order.vendor == "Vendor B");
    CHECK(order.quantity == 2);
    CHECK(order.preTaxCost == Money::of(150));
    CHECK(order.postTaxCost == Money::of(150));
    CHECK(catalog.holdings("B1") == 2);
    CHECK(budget.spent() == Money::of(150));
}

static void testVendorPriceUpdate() {
    Catalog catalog;
    addBook(catalog, "B1", "Algorithms", Money::of(100));

    Budget budget(Money::of(1000));
    AcquisitionManager acq(catalog, budget);

    acq.addVendorOffer("B1", "Vendor A", Money::of(90));
    acq.addVendorOffer("B1", "Vendor B", Money::of(80));

    // Registering the same vendor again should update its offer.
    acq.addVendorOffer("B1", "Vendor A", Money::of(70));

    CHECK(acq.cheapestVendor("B1").vendor == "Vendor A");
    CHECK(acq.quote("B1", 1) == Money::of(70));
}

static void testCataloguePriceFallback() {
    Catalog catalog;
    addBook(catalog, "B1", "Algorithms", Money::of(100));

    Budget budget(Money::of(1000));
    AcquisitionManager acq(catalog, budget);

    CHECK(acq.quote("B1", 2) == Money::of(200));

    const PurchaseRecord& order = acq.purchase("B1", 2);

    CHECK(order.approved);
    CHECK(order.vendor.empty());
    CHECK(order.preTaxCost == Money::of(200));
}

static void testInvalidVendorOffers() {
    Catalog catalog;
    addBook(catalog, "B1", "Algorithms", Money::of(100));

    Budget budget(Money::of(1000));
    AcquisitionManager acq(catalog, budget);

    CHECK_THROWS(
        acq.addVendorOffer("UNKNOWN", "Vendor A", Money::of(50)),
        std::invalid_argument
    );

    CHECK_THROWS(
        acq.addVendorOffer("B1", "", Money::of(50)),
        std::invalid_argument
    );

    CHECK_THROWS(
        acq.addVendorOffer("B1", "Vendor A", Money::of(-1)),
        std::invalid_argument
    );

    CHECK_THROWS(
        acq.cheapestVendor("B1"),
        std::invalid_argument
    );
}

static void testBatchUsesCheapestVendors() {
    Catalog catalog;
    addBook(catalog, "B1", "Algorithms", Money::of(100));
    addBook(catalog, "B2", "Databases", Money::of(120));

    Budget budget(Money::of(1000));
    AcquisitionManager acq(catalog, budget);

    acq.addVendorOffer("B1", "Vendor A", Money::of(90));
    acq.addVendorOffer("B1", "Vendor B", Money::of(80));

    acq.addVendorOffer("B2", "Vendor C", Money::of(110));
    acq.addVendorOffer("B2", "Vendor D", Money::of(95));

    const auto results = acq.processBatch({
        {"B1", 2},
        {"B2", 1}
    });

    CHECK(results.size() == 2);
    CHECK(results[0].approved);
    CHECK(results[1].approved);

    CHECK(results[0].vendor == "Vendor B");
    CHECK(results[0].preTaxCost == Money::of(160));

    CHECK(results[1].vendor == "Vendor D");
    CHECK(results[1].preTaxCost == Money::of(95));

    CHECK(catalog.holdings("B1") == 2);
    CHECK(catalog.holdings("B2") == 1);
    CHECK(budget.spent() == Money::of(255));

    CHECK(acq.history().size() == 2);
    CHECK(acq.history()[0].vendor == "Vendor B");
    CHECK(acq.history()[1].vendor == "Vendor D");
}

// ============================================================
// Main
// ============================================================



// ============================================================
// Q13 - Search by author, ISBN/ISSN, and publication year
// ============================================================

static void testCatalogSearchesQ13() {
    Catalog catalog;

    catalog.emplace<Book>(
        "B1", "Clean Code",
        std::vector<std::string>{"Robert C. Martin", "Author Two"},
        "978-0132350884", "Prentice Hall", 2008, Money::of(100)
    );

    catalog.emplace<Book>(
        "B2", "Design Patterns",
        std::vector<std::string>{"Erich Gamma", "Richard Helm"},
        "978-0201633610", "Addison-Wesley", 1994, Money::of(120)
    );

    catalog.emplace<EBook>(
        "E1", "Clean Architecture",
        std::vector<std::string>{"Robert C. Martin"},
        "978-0134494166", "Prentice Hall", 2017, Money::of(20),
        "https://example.com/clean-architecture",
        LicenseModel::Perpetual, Money::of(0), "PDF", false
    );

    catalog.emplace<Journal>(
        "J1", "ACM Computing Surveys", "1234-5678", 12,
        "ACM", 2022, Money::of(50)
    );

    // Search by author: case-insensitive partial matches.
    auto authorResults = catalog.searchAuthor("robert c. martin");
    CHECK(authorResults.size() == 2);

    authorResults = catalog.searchAuthor("Erich");
    CHECK(authorResults.size() == 1);
    CHECK(authorResults[0]->id() == "B2");

    CHECK(catalog.searchAuthor("No Such Author").empty());

    // Search by ISBN/ISSN: books, e-books, and journals.
    auto isbnResults = catalog.searchIsbnIssn("978-0132350884");
    CHECK(isbnResults.size() == 1);
    CHECK(isbnResults[0]->id() == "B1");

    auto issnResults = catalog.searchIsbnIssn("1234-5678");
    CHECK(issnResults.size() == 1);
    CHECK(issnResults[0]->id() == "J1");

    auto partialResults = catalog.searchIsbnIssn("978-013");
    CHECK(partialResults.size() == 2);

    CHECK(catalog.searchIsbnIssn("UNKNOWN-ID").empty());

    // Publication-year range is inclusive at both endpoints.
    auto yearResults = catalog.searchYearRange(2008, 2017);
    CHECK(yearResults.size() == 2);

    yearResults = catalog.searchYearRange(1994, 1994);
    CHECK(yearResults.size() == 1);
    CHECK(yearResults[0]->id() == "B2");

    yearResults = catalog.searchYearRange(2023, 2025);
    CHECK(yearResults.empty());

    // An inverted range is invalid.
    CHECK_THROWS(
        catalog.searchYearRange(2020, 2000),
        std::invalid_argument
    );
}



// ============================================================
// Q14 - Lending: print copies and electronic-resource sessions
// ============================================================
static void testLendingQ14() {
    Catalog catalog;

    catalog.emplace<Book>(
        "L14-B1",
        "Lending Test Book",
        std::vector<std::string>{"Test Author"},
        "ISBN-L14",
        "Test Publisher",
        2026,
        Money::of(100)
    );

    catalog.addHoldings("L14-B1", 2);

    catalog.emplace<EBook>(
        "L14-E1",
        "Lending Test EBook",
        std::vector<std::string>{"Test Author"},
        "ISBN-E14",
        "Test Publisher",
        2026,
        Money::of(100),
        "https://example.test/ebook"
    );

    catalog.addHoldings("L14-E1", 1);

    LendingManager lending(catalog);

    // Print-copy borrowing respects the number of copies held.
    CHECK(lending.availableCopies("L14-B1") == 2);
    lending.borrow("L14-B1", "P1");
    CHECK(lending.activeLoans("L14-B1") == 1);
    CHECK(lending.availableCopies("L14-B1") == 1);

    lending.borrow("L14-B1", "P2");
    CHECK(lending.activeLoans("L14-B1") == 2);
    CHECK(lending.availableCopies("L14-B1") == 0);

    CHECK_THROWS(
        lending.borrow("L14-B1", "P3"),
        LendingCapacityError
    );

    // Returning a copy restores availability.
    lending.returnCopy("L14-B1", "P1");
    CHECK(lending.activeLoans("L14-B1") == 1);
    CHECK(lending.availableCopies("L14-B1") == 1);

    CHECK_THROWS(
        lending.returnCopy("L14-B1", "P1"),
        LoanNotFoundError
    );

    lending.returnCopy("L14-B1", "P2");
    CHECK(lending.activeLoans("L14-B1") == 0);
    CHECK(lending.availableCopies("L14-B1") == 2);

    // Electronic sessions respect the licensed-seat count.
    CHECK(lending.availableSeats("L14-E1") == 1);
    lending.openSession("L14-E1", "P1");
    CHECK(lending.activeSessions("L14-E1") == 1);
    CHECK(lending.availableSeats("L14-E1") == 0);

    CHECK_THROWS(
        lending.openSession("L14-E1", "P2"),
        LendingCapacityError
    );

    // Closing a session releases its licensed seat.
    lending.closeSession("L14-E1", "P1");
    CHECK(lending.activeSessions("L14-E1") == 0);
    CHECK(lending.availableSeats("L14-E1") == 1);

    CHECK_THROWS(
        lending.closeSession("L14-E1", "P1"),
        SessionNotFoundError
    );

    // Reject operations on the wrong resource type.
    CHECK_THROWS(
        lending.openSession("L14-B1", "P1"),
        std::invalid_argument
    );

    CHECK_THROWS(
        lending.borrow("L14-E1", "P1"),
        std::invalid_argument
    );

    // Reject an empty patron ID.
    CHECK_THROWS(
        lending.borrow("L14-B1", ""),
        std::invalid_argument
    );

    // Unknown resource IDs are rejected.
    CHECK_THROWS(
        lending.borrow("UNKNOWN-L14", "P1"),
        NotFoundError
    );
}


// ============================================================
// Q15 - Money currency compatibility
// ============================================================
static void testMoneyCurrencyQ15() {
    const Money inr100 = Money::of(100, 0, "INR");
    const Money inr50 = Money::of(50, 0, "INR");
    const Money usd100 = Money::of(100, 0, "USD");

    // Currency codes are stored and preserved.
    CHECK(inr100.currencyCode() == "INR");
    CHECK(usd100.currencyCode() == "USD");
    CHECK(Money::of(1).currencyCode() == "INR");
    CHECK(Money::fromMinor(500, "USD").currencyCode() == "USD");

    // Arithmetic using the same currency remains valid.
    CHECK((inr100 + inr50).minorUnits() == 15000);
    CHECK((inr100 - inr50).minorUnits() == 5000);
    CHECK((inr100 * 2).currencyCode() == "INR");
    CHECK((usd100 * 2).currencyCode() == "USD");

    // In-place arithmetic must reject different currencies.
    Money total = inr100;
    CHECK_THROWS(total += usd100, std::invalid_argument);

    total = inr100;
    CHECK_THROWS(total -= usd100, std::invalid_argument);

    // Binary arithmetic must reject different currencies.
    CHECK_THROWS(inr100 + usd100, std::invalid_argument);
    CHECK_THROWS(inr100 - usd100, std::invalid_argument);

    // All comparison operators must reject different currencies.
    CHECK_THROWS(inr100 == usd100, std::invalid_argument);
    CHECK_THROWS(inr100 != usd100, std::invalid_argument);
    CHECK_THROWS(inr100 < usd100, std::invalid_argument);
    CHECK_THROWS(inr100 <= usd100, std::invalid_argument);
    CHECK_THROWS(inr100 > usd100, std::invalid_argument);
    CHECK_THROWS(inr100 >= usd100, std::invalid_argument);

    // An empty currency code is invalid.
    CHECK_THROWS(
        Money::of(1, 0, ""),
        std::invalid_argument
    );

    CHECK_THROWS(
        Money::fromMinor(100, ""),
        std::invalid_argument
    );
}

int main() {

    testMoney();
    testMoneyCurrencyQ15();

    testResourcesAndCost();

    testJournal();

    testEBook();

    testAudioBookAndThesis();

    testBookPricing();

    testBulkDiscounts();

    testCatalog();
    testCatalogSearchesQ13();
    testLendingQ14();

    testBudget();

    testAcquisition();

    testTaxes();

    testCancellation();

    testDepartments();

    testBudgetRollover();

    testBatchAllOrNothing();

    testCheapestVendorAndPurchase();
    testVendorPriceUpdate();
    testCataloguePriceFallback();
    testInvalidVendorOffers();
    testBatchUsesCheapestVendors();

    std::cout
        << "\n"
        << (g_checks - g_failures)
        << "/"
        << g_checks
        << " checks passed\n";

    if (g_failures == 0) {
        std::cout
            << "All tests passed.\n";
        return 0;
    }

    std::cout
        << g_failures
        << " checks failed.\n";

    return 1;
}
