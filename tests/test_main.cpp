#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "bookmgmt/Acquisition.h"
#include "bookmgmt/AudioBook.h"
#include "bookmgmt/Book.h"
#include "bookmgmt/Budget.h"
#include "bookmgmt/Catalog.h"
#include "bookmgmt/Exceptions.h"
#include "bookmgmt/EBook.h"
#include "bookmgmt/ElectronicResource.h"
#include "bookmgmt/Journal.h"
#include "bookmgmt/Money.h"
#include "bookmgmt/Thesis.h"

using namespace bookmgmt;

static int g_checks = 0;
static int g_failures = 0;

#define CHECK(cond)                                                        \
    do {                                                                   \
        ++g_checks;                                                        \
        if (!(cond)) {                                                     \
            ++g_failures;                                                  \
            std::cerr << "CHECK failed: " << #cond                         \
                      << " at " << __FILE__ << ":" << __LINE__ << "\n";   \
        }                                                                  \
    } while (false)

#define CHECK_THROWS(expr, exception_type)                                  \
    do {                                                                    \
        ++g_checks;                                                         \
        bool caught = false;                                                \
        try {                                                               \
            (expr);                                                         \
        } catch (const exception_type&) {                                   \
            caught = true;                                                  \
        } catch (...) {                                                     \
        }                                                                   \
        if (!caught) {                                                      \
            ++g_failures;                                                   \
            std::cerr << "CHECK_THROWS failed: " << #expr                 \
                      << " at " << __FILE__ << ":" << __LINE__ << "\n";    \
        }                                                                   \
    } while (false)

/*
 * Basic Money tests.
 */
static void testMoney() {
    Money a = Money::of(10);
    Money b = Money::of(5, 50);

    CHECK(a == Money::of(10));
    CHECK(b == Money::of(5, 50));

    CHECK(a + b == Money::of(15, 50));
    CHECK(a - b == Money::of(4, 50));

    CHECK(a * 3 == Money::of(30));
    CHECK(3 * a == Money::of(30));

    CHECK(Money::of(0).isZero());
    CHECK(Money::of(-1).isNegative());

    CHECK(Money::of(12, 34).toString() == "12.34");
}

/*
 * Tests for Resource, Book and ElectronicResource.
 */
static void testResourcesAndCost() {
    Book book(
        "B1",
        "Clean Code",
        {"Robert C. Martin"},
        "978-0132350884",
        "Prentice Hall",
        2008,
        Money::of(100)
    );

    CHECK(book.category() == ResourceCategory::Book);
    CHECK(book.costFor(1) == Money::of(100));
    CHECK(book.costFor(3) == Money::of(300));

    ElectronicResource resource(
        "R1",
        "Digital Library",
        "Publisher",
        2026,
        Money::of(20),
        "https://example.com"
    );

    CHECK(resource.costFor(2) == Money::of(40));
    CHECK(resource.isDigital());
}

/*
 * Q1: Journal
 */
static void testJournal() {
    Journal journal(
        "J1",
        "Computer Journal",
        "1234-5678",
        12,
        "Publisher",
        2026,
        Money::of(100),
        2
    );

    CHECK(journal.category() == ResourceCategory::Journal);
    CHECK(journal.issn() == "1234-5678");
    CHECK(journal.issuesPerYear() == 12);
    CHECK(journal.subscriptionYears() == 2);

    // 100 * 2 copies * 2 years = 400.
    CHECK(journal.costFor(2) == Money::of(400));

    CHECK_THROWS(
        Journal(
            "J2",
            "Invalid Journal",
            "0000",
            12,
            "Publisher",
            2026,
            Money::of(100),
            0
        ),
        std::invalid_argument
    );

    std::ostringstream os;
    journal.print(os);

    CHECK(
        os.str().find("issn: 1234-5678")
        != std::string::npos
    );

    CHECK(
        os.str().find("issues per year: 12")
        != std::string::npos
    );
}

/*
 * Q2: EBook
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

    CHECK(e.category() == ResourceCategory::EBook);
    CHECK(e.authors().size() == 1);
    CHECK(e.isbn() == "978-0132350884");
    CHECK(e.fileFormat() == "PDF");
    CHECK(e.drmProtected());

    // Platform fee = 100.
    // Unit price = 20.
    // 5 seats = 100 + 20 * 5 = 200.
    CHECK(e.costFor(5) == Money::of(200));

    const ElectronicResource& er = e;

    CHECK(er.costFor(5) == Money::of(200));

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

    CHECK(e2.category() == ResourceCategory::EBook);
    CHECK(e2.authors().size() == 2);
    CHECK(e2.isbn() == "978-0201633610");
    CHECK(e2.fileFormat() == "EPUB");
    CHECK(!e2.drmProtected());

    CHECK(e2.costFor(2) == Money::of(30));
}

/*
 * Q3: AudioBook and Thesis
 */
static void testAudioBookAndThesis() {
    /*
     * Actual AudioBook constructor order:
     *
     * id
     * title
     * narrator
     * duration
     * publisher
     * year
     * price
     * access URL
     * license
     * platform fee
     */
    AudioBook audio(
        "A1",
        "C++ Audio Course",
        "Narrator",
        120,
        "Publisher",
        2026,
        Money::of(20),
        "https://example.com/audio",
        LicenseModel::Perpetual,
        Money::of(10)
    );

    CHECK(audio.category() == ResourceCategory::AudioBook);
    CHECK(audio.narrator() == "Narrator");
    CHECK(audio.durationMinutes() == 120);

    // Platform fee = 10.
    // 2 seats * 20 = 40.
    // Total = 50.
    CHECK(audio.costFor(2) == Money::of(50));

    CHECK_THROWS(
        AudioBook(
            "A2",
            "Invalid Audio",
            "Narrator",
            0,
            "Publisher",
            2026,
            Money::of(20),
            "url",
            LicenseModel::Perpetual,
            Money::of(0)
        ),
        std::invalid_argument
    );

    Thesis thesis(
        "T1",
        "Machine Learning Thesis",
        "IIIT Delhi",
        "M.Tech",
        "Professor",
        "IIIT Delhi",
        2026,
        Money::of(0)
    );

    CHECK(thesis.category() == ResourceCategory::Thesis);
    CHECK(thesis.university() == "IIIT Delhi");
    CHECK(thesis.degree() == "M.Tech");
    CHECK(thesis.supervisor() == "Professor");

    CHECK(thesis.costFor(1) == Money::of(0));
}

/*
 * Q4: Hardcover pricing
 *
 * Paperback books use their listed unit price.
 * Hardcover books cost 20% more than their listed unit price.
 */
static void testBookPricing() {
    Book paperback(
        "BP1",
        "Paperback Book",
        {"Author"},
        "ISBN-P",
        "Publisher",
        2026,
        Money::of(100),
        1,
        Binding::Paperback
    );

    CHECK(paperback.binding() == Binding::Paperback);
    CHECK(paperback.costFor(1) == Money::of(100));
    CHECK(paperback.costFor(3) == Money::of(300));

    Book hardcover(
        "BH1",
        "Hardcover Book",
        {"Author"},
        "ISBN-H",
        "Publisher",
        2026,
        Money::of(100),
        1,
        Binding::Hardcover
    );

    CHECK(hardcover.binding() == Binding::Hardcover);

    // 100 + 20% = 120.
    CHECK(hardcover.costFor(1) == Money::of(120));

    // 120 * 3 = 360.
    CHECK(hardcover.costFor(3) == Money::of(360));

    CHECK_THROWS(
        hardcover.costFor(0),
        std::invalid_argument
    );
}

/*
 * Q5: Bulk discounts
 *
 * Print items:
 *   10 or more copies -> 10% discount.
 *
 * Electronic resources:
 *   First 50 seats -> full price.
 *   Every seat beyond 50 -> half price.
 */
static void testBulkDiscounts() {
    Book paperback(
        "Q5-B",
        "Bulk Paperback",
        {"Author"},
        "ISBN-Q5-B",
        "Publisher",
        2026,
        Money::of(100),
        1,
        Binding::Paperback
    );

    CHECK(paperback.costFor(9) == Money::of(900));
    CHECK(paperback.costFor(10) == Money::of(900));
    CHECK(paperback.costFor(20) == Money::of(1800));

    Book hardcover(
        "Q5-H",
        "Bulk Hardcover",
        {"Author"},
        "ISBN-Q5-H",
        "Publisher",
        2026,
        Money::of(100),
        1,
        Binding::Hardcover
    );

    // Hardcover price = 120.
    //
    // 9 copies  = 1080.
    // 10 copies = 1200 - 10% = 1080.
    // 20 copies = 2400 - 10% = 2160.
    CHECK(hardcover.costFor(9) == Money::of(1080));
    CHECK(hardcover.costFor(10) == Money::of(1080));
    CHECK(hardcover.costFor(20) == Money::of(2160));

    Journal journal(
        "Q5-J",
        "Bulk Journal",
        "ISSN-Q5",
        12,
        "Publisher",
        2026,
        Money::of(100),
        2
    );

    CHECK(journal.costFor(9) == Money::of(1800));
    CHECK(journal.costFor(10) == Money::of(1800));
    CHECK(journal.costFor(20) == Money::of(3600));

    EBook ebook(
        "Q5-E",
        "Bulk EBook",
        {"Author"},
        "ISBN-Q5-E",
        "Publisher",
        2026,
        Money::of(100),
        "https://example.com/ebook",
        LicenseModel::AnnualSubscription,
        Money::of(0),
        "PDF",
        true
    );

    CHECK(ebook.costFor(49) == Money::of(4900));
    CHECK(ebook.costFor(50) == Money::of(5000));
    CHECK(ebook.costFor(51) == Money::of(5050));
    CHECK(ebook.costFor(60) == Money::of(5500));

    ElectronicResource electronic(
        "Q5-R",
        "Bulk Electronic Resource",
        "Publisher",
        2026,
        Money::of(100),
        "url",
        LicenseModel::Perpetual,
        Money::of(500)
    );

    CHECK(electronic.costFor(50) == Money::of(5500));
    CHECK(electronic.costFor(60) == Money::of(6000));

    CHECK_THROWS(
        paperback.costFor(0),
        std::invalid_argument
    );

    CHECK_THROWS(
        electronic.costFor(0),
        std::invalid_argument
    );
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

    CHECK(
        c.byCategory(ResourceCategory::ElectronicResource).size()
        == 1
    );

    CHECK(
        c.where([](const Resource& r) {
            return r.isDigital();
        }).size() == 1
    );
}

static void testBudget() {
    Budget b(Money::of(1000));

    b.setQuota(
        ResourceCategory::Book,
        {5, Money::of(500)}
    );

    CHECK(b.total() == Money::of(1000));
    CHECK(b.spent() == Money::of(0));
    CHECK(b.remaining() == Money::of(1000));

    CHECK(
        b.check(
            ResourceCategory::Book,
            2,
            Money::of(200)
        ).empty()
    );

    b.commit(
        ResourceCategory::Book,
        2,
        Money::of(200)
    );

    CHECK(b.spent() == Money::of(200));

    CHECK(
        b.usageFor(ResourceCategory::Book).units
        == 2
    );

    CHECK(
        b.usageFor(ResourceCategory::Book).spent
        == Money::of(200)
    );

    CHECK_THROWS(
        b.commit(
            ResourceCategory::Book,
            4,
            Money::of(400)
        ),
        QuotaExceededError
    );

    CHECK(b.spent() == Money::of(200));

    CHECK(
        b.check(
            ResourceCategory::Book,
            1,
            Money::of(400)
        ).empty() == false
    );
}

/*
 * Existing acquisition tests.
 */
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

    CHECK(
        c.holdings("R1") == 10
        && c.holdings("B1") == 3
    );

    CHECK(acq.history().size() == 6);
}

/*
 * Q6: Taxes
 *
 * Print and electronic resources have separately configurable tax rates.
 *
 * The purchase record stores:
 *   1. pre-tax cost
 *   2. tax
 *   3. post-tax cost
 *
 * Budget and category quota checks use the post-tax cost.
 */
static void testTaxes() {
    Catalog c;

    /*
     * Print book:
     *
     * Unit price = 100.00
     */
    c.emplace<Book>(
        "T-B1",
        "Taxed Book",
        std::vector<std::string>{"Author"},
        "ISBN-TAX",
        "Publisher",
        2026,
        Money::of(100)
    );

    /*
     * Electronic resource:
     *
     * Platform fee = 20.00
     * Price per seat = 10.00
     */
    c.emplace<ElectronicResource>(
        "T-R1",
        "Taxed Database",
        "Publisher",
        2026,
        Money::of(10),
        "url",
        LicenseModel::AnnualSubscription,
        Money::of(20)
    );

    Budget b(Money::of(1000));

    AcquisitionManager acq(c, b);

    /*
     * Configure separate tax rates.
     *
     * Print       = 10%
     * Electronic  = 20%
     */
    acq.setPrintTaxRate(10.0);
    acq.setElectronicTaxRate(20.0);

    CHECK(acq.printTaxRate() == 10.0);
    CHECK(acq.electronicTaxRate() == 20.0);

    /*
     * Print purchase:
     *
     * 2 books
     *
     * Pre-tax  = 100 * 2 = 200
     * Tax      = 10% of 200 = 20
     * Post-tax = 220
     */
    const auto& bookRecord = acq.purchase("T-B1", 2);

    CHECK(bookRecord.approved);
    CHECK(bookRecord.preTaxCost == Money::of(200));
    CHECK(bookRecord.tax == Money::of(20));
    CHECK(bookRecord.postTaxCost == Money::of(220));

    /*
     * `cost` is retained for compatibility with the original
     * PurchaseRecord API and represents the final amount charged.
     */
    CHECK(bookRecord.cost == Money::of(220));

    /*
     * Electronic purchase:
     *
     * 5 seats
     *
     * Pre-tax = 20 + (10 * 5)
     *         = 70
     *
     * Tax      = 20% of 70
     *          = 14
     *
     * Post-tax = 84
     */
    const auto& electronicRecord = acq.purchase("T-R1", 5);

    CHECK(electronicRecord.approved);
    CHECK(electronicRecord.preTaxCost == Money::of(70));
    CHECK(electronicRecord.tax == Money::of(14));
    CHECK(electronicRecord.postTaxCost == Money::of(84));
    CHECK(electronicRecord.cost == Money::of(84));

    /*
     * Total spending includes tax:
     *
     * Book       = 220
     * Electronic = 84
     * Total      = 304
     */
    CHECK(acq.totalSpent() == Money::of(304));
    CHECK(b.spent() == Money::of(304));

    /*
     * Q6: verify quotas are checked against POST-TAX cost.
     *
     * Book:
     *   Pre-tax  = 100
     *   Tax      = 10
     *   Post-tax = 110
     *
     * Quota:
     *   Maximum spending = 105
     *
     * Pre-tax 100 would fit.
     * Post-tax 110 does not fit.
     *
     * Therefore the purchase must be rejected.
     */
    Budget quotaBudget(Money::of(1000));

    quotaBudget.setQuota(
        ResourceCategory::Book,
        {10, Money::of(105)}
    );

    AcquisitionManager quotaAcq(c, quotaBudget);

    quotaAcq.setPrintTaxRate(10.0);

    std::string why;

    CHECK(
        !quotaAcq.canPurchase("T-B1", 1, &why)
        && !why.empty()
    );

    CHECK_THROWS(
        quotaAcq.purchase("T-B1", 1),
        QuotaExceededError
    );

    /*
     * Rejected purchase must not change the budget
     * or the holdings.
     */
    CHECK(quotaBudget.spent() == Money::of(0));
    CHECK(c.holdings("T-B1") == 2);

    /*
     * Test the purchase report.
     */
    std::ostringstream report;

    acq.printReport(report);

    const std::string reportText = report.str();

    CHECK(
        reportText.find("pre-tax: 200.00")
        != std::string::npos
    );

    CHECK(
        reportText.find("tax: 20.00")
        != std::string::npos
    );

    CHECK(
        reportText.find("post-tax: 220.00")
        != std::string::npos
    );

    CHECK(
        reportText.find("pre-tax: 70.00")
        != std::string::npos
    );

    CHECK(
        reportText.find("tax: 14.00")
        != std::string::npos
    );

    CHECK(
        reportText.find("post-tax: 84.00")
        != std::string::npos
    );

    CHECK(
        reportText.find("Total spent (post-tax): 304.00")
        != std::string::npos
    );

    /*
     * Negative tax rates are invalid.
     */
    CHECK_THROWS(
        acq.setPrintTaxRate(-1.0),
        std::invalid_argument
    );

    CHECK_THROWS(
        acq.setElectronicTaxRate(-1.0),
        std::invalid_argument
    );
}

int main() {
    testMoney();
    testResourcesAndCost();
    testJournal();
    testEBook();
    testAudioBookAndThesis();
    testBookPricing();
    testBulkDiscounts();
    testCatalog();
    testBudget();
    testAcquisition();
    testTaxes();

    std::cout
        << (g_checks - g_failures)
        << "/"
        << g_checks
        << " checks passed\n";

    return g_failures == 0 ? 0 : 1;
}
