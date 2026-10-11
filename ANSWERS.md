# OOPD Assignment 2 — Answers

## Q1. Journal

Implemented `Journal` as a subclass of `Resource`. It stores the ISSN, issues per year, and subscription duration. The subscription duration must be at least one year. The total cost accounts for the annual subscription price, quantity, and subscription duration. Journal is integrated into the resource-category system.

## Q2. EBook

Implemented `EBook` as a subclass of `ElectronicResource`. It stores authors, ISBN, file format, and DRM status. It reuses electronic-resource pricing and prints inherited details before its own fields.

**Avoiding duplicated metadata:** Both `Book` and `EBook` require author and ISBN metadata. Duplicating these fields and their accessors can make maintenance harder. A reusable metadata component or shared abstraction could hold the common information while allowing print and electronic resources to retain their distinct pricing and behaviour.

## Q3. AudioBook and Thesis

`AudioBook` derives from `ElectronicResource` because it is delivered electronically and can reuse electronic-resource pricing behaviour. It adds narrator and duration fields.

`Thesis` derives from `Resource` because it is a distinct catalogue resource with university, degree, and supervisor metadata. Its pricing behaviour can reflect the requirement that theses are free of cost.

## Q4. Hardcover Pricing

Implemented a 20% price increase for hardcover books.

## Q5. Bulk Discounts

Implemented a 10% discount for print purchases of at least 10 copies. For electronic resources, seats beyond the first 50 are charged at half price.

## Q6. Taxes

Implemented configurable tax rates for print and electronic resources. Reports include pre-tax and post-tax totals, and budget and quota checks use post-tax costs.

## Q7. Category Title Limits

Implemented per-category limits on the number of different titles that may be purchased.

## Q8. Cancellation

Implemented cancellation with budget and quota refunds and a reduction in catalogue holdings. Original order history is preserved, and a cancellation record is added.

## Q9. Department Budgets

Implemented department-specific budgets and quotas. Each purchase request identifies the department whose limits apply.

## Q10. Year-End Rollover

Implemented a configurable percentage of unspent budget to be carried into the following year.

## Q11. Batch Processing

Implemented all-or-nothing batch processing so that a rejected request prevents the batch from being committed when this mode is enabled.

## Q12. Vendors

Implemented multiple vendor offers with different prices. The cheapest available offer is selected, and the vendor is stored in the order record.

## Q13. Catalogue Searches

Implemented searches by author, ISBN/ISSN, and publication-year range.

## Q14. Lending

Implemented borrowing and returning print copies, limited by copies held. Implemented opening and closing electronic-resource sessions, limited by licensed seats.

## Q15. Currency Support

Added a currency code to `Money`. Arithmetic and comparisons between different currencies throw an exception.

## Q16. Assertions

Replaced the test macros with `assert` while preserving the intended checks, including checks that expected exceptions are thrown.

## General Implementation and Testing

- Used the C++17 project structure.
- Added tests in `tests/test_main.cpp` for the implemented features.
- Extended `examples/demo.cpp` to demonstrate implemented functionality.
- Built and tested the project using CMake and CTest.
- Used compiler warning flags including `-Wall`, `-Wextra`, and `-Wpedantic`.

## General WSL and Bash Commands Used

The assignment was developed and tested using WSL (Ubuntu). The following commands were used to navigate the project, inspect source files, build and test the application, and manage the Git repository.

### 1. Directory Navigation and File Inspection

```bash
pwd
cd /mnt/c/Users/Soham/Downloads/Assignment-2/BookManagement
ls
ls -l
cd ..
cat README.md
cat CMakeLists.txt
```

- `pwd` displays the current working directory.
- `cd` changes the current directory.
- `ls` and `ls -l` list files and directories.
- `cat` displays the contents of a file.

### 2. Searching and Inspecting Source Code

```bash
grep -n "pattern" filename
grep -n -A 25 "pattern" filename
```

- `grep -n` searches for a pattern and displays matching line numbers.
- `grep -n -A 25` displays the matching line and the 25 lines following it.

### 3. Building and Testing the Project

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

- `cmake -S . -B build` configures the project and generates build files in the `build` directory.
- `cmake --build build` compiles the project.
- `ctest --test-dir build --output-on-failure` runs the registered tests and displays details if any tests fail.

### 4. Running the Demo

```bash
./build/bookmgmt_demo
```

This command runs the demo executable if it is located at the specified path.

### 5. Git and GitHub Commands

```bash
git status
git remote -v
git log --oneline
git add ANSWERS.md
git commit -m "Add assignment answers"
git push origin main
```

- `git status` displays modified, staged, and untracked files.
- `git remote -v` displays the configured remote repositories.
- `git log --oneline` displays the commit history in a concise format.
- `git add` stages changes for a commit.
- `git commit` records the staged changes.
- `git push origin main` pushes local commits to the remote `main` branch.

These commands supported project navigation, source-code inspection, compilation, testing, and version control during the assignment.

## LLM Usage Declaration

LLM was used as a supplementary tool during the development of this assignment which includes understanding the requirements, discussing object-oriented design, debugging C++ code, preparing tests, using development tools, and documenting the implementation.

### Representative Prompts

The following prompts summarize the types of requests made during development. They are representative examples and are not necessarily verbatim reproductions of the original conversation history.

**Understanding the assignment**
   - "Explain the assignment requirements and help me understand what needs to be implemented for each question."
   - "Explain the given code and the object-oriented programming concepts involved in detail."

**Class design and implementation**
   - "Help me implement the required classes using inheritance and appropriate object-oriented design."
   - "Explain how inheritance, composition, and aggregation apply to this design."
   - "Help me implement the required pricing rules, discounts, taxes, budgets, quotas, and acquisition features."

**Debugging and testing**
   - "Help me identify and fix compilation errors in my C++ code."
   - "Explain the errors in my code and provide the corrected implementation."
   - "Help me add tests for the required functionality and verify that the existing tests continue to pass."
   - "Help me build the project using CMake and run the tests using CTest."

**Catalogue and lending functionality**
   - "Help me implement catalogue searches by author, ISBN/ISSN, and publication-year range."
   - "Help me implement borrowing and returning print resources and opening and closing electronic-resource sessions."

**Currency support and assertions**
   - "Help me add currency codes to the Money class and prevent arithmetic and comparisons between different currencies."
   - "Help me replace the test macros with assert while preserving the intended checks."

**Build tools and version control**
   - "Give me the WSL Bash commands to navigate the project, compile the code, and run the tests."
   - "Explain Git commands for checking changes, staging files, committing changes, and pushing to GitHub."

**Documentation**
   - "Help me prepare ANSWERS.md summarizing the implementation of each assignment question."
   - "Help me document the development workflow and LLM assistance used during the assignment."

### Declaration of Responsibility

LLM assistance was used to support understanding, implementation, debugging, testing, and documentation. The author remains responsible for reviewing the generated suggestions, verifying the implementation, and ensuring that the submitted work complies with the assignment requirements.
