# OOP Concepts Used in the ATM Project

This section summarizes how the six core OOP concepts taught in CSE201 are applied across our banking and ATM simulation.

---

## 1. Encapsulation
Sensitive data and internal invariants are kept `private` or `protected` and can only be modified through validated public methods:
- **`Account`**: Stores `balance`, `password`, `accountNumber`, and `history` privately. The password is never returned by a getter; external code can only check it through `checkPassword()`. Balance updates go through `deposit()` and `withdraw()`, which prevent negative balances or non-positive amounts.
- **`CashInventory`**: Encapsulates the internal denomination map (`bills`). Direct modification is not allowed; callers must use `addBills()`, `removeBills()`, `tryDispense()`, or `dispense()`, which ensure bill counts never become negative and that greedy dispensing is atomic (all-or-nothing).
- **`Session`**: Keeps `wrongPasswordAttempts`, `withdrawalCount`, `active`, and `authorized` private so session rules (maximum 3 wrong password attempts, maximum 3 withdrawals per session) cannot be bypassed.
- **`ATM`**: Protects `serialNumber`, `primaryBankName`, `languageMode`, and `inventory`, validating the 6-digit serial number on construction and preventing unilingual ATMs from switching away from English in `setLanguage()`.

---

## 2. Abstraction
Abstract base classes define shared interfaces while hiding concrete implementation details from callers:
- **`ATM`**: Declares pure virtual methods `isCardValid(const Card&) const = 0` and `getType() const = 0`. `main.cpp`, `Session`, `Snapshot`, and `AdminMenu` all work through `ATM&` or `ATM*` without knowing whether the machine is a single-bank or multi-bank ATM.
- **`Transaction`**: Declares pure virtual methods `execute() = 0`, `type() const = 0`, and `describe() const = 0`, while factoring out shared logic (`commit()` for unique IDs, `tr()` for localized messages, `issuingBank()`). `Session` runs every transaction through the same `Transaction` interface.
- **`FeePolicy` & `Messages`**: Separate fee calculation rules and bilingual UI text tables from transaction logic so changes to fees or wording do not affect core banking flows.

---

## 3. Inheritance
Related classes share common state and behavior through inheritance hierarchies:
- **`SingleBankATM` and `MultiBankATM` inherit from `ATM`**: Both inherit serial number, primary bank, language settings, and `CashInventory`, and specialize only card acceptance rules and type labels.
- **`DepositTransaction`, `WithdrawalTransaction`, `CashTransferTransaction`, and `AccountTransferTransaction` inherit from `Transaction`**: All four share the ATM/Account references, amount/fee fields, and static ID generator.
- **`AdminCard` inherits from `Card`**: Extends a regular card to mark administrative privileges (`isAdmin() == true`).
- **Exception hierarchy**: `InvalidCardException`, `WrongPasswordException`, `InsufficientFundsException`, `InsufficientCashException`, `LimitExceededException`, and `InvalidInputException` all inherit from `ATMException` (which inherits from `std::runtime_error`).

---

## 4. Polymorphism
Virtual functions and base-class pointers/references allow runtime dispatch based on the actual object type:
- **`ATM*` in `main.cpp`**: Both `SingleBankATM` and `MultiBankATM` objects are stored together in `std::vector<ATM*>`. When `Session` calls `atm.isCardValid(card)`, C++ dispatches dynamically to either `SingleBankATM::isCardValid` (primary bank only) or `MultiBankATM::isCardValid` (any bank).
- **`Card&` in `Session::run`**: Both `Card` and `AdminCard` are passed as `const Card&`. Calling `card.isAdmin()` dynamically routes admin cards to `AdminMenu::run()` while normal cards proceed to password authentication.
- **`std::shared_ptr<Transaction>` in `Session`**: `Session::runMenuLoop` creates whichever concrete transaction the user chose, then calls `tx->execute()`, `tx->type()`, and `tx->describe()` polymorphically. `Session::printSummary()` iterates over the polymorphic list to print the end-of-session receipt.
- **Virtual destructors**: `ATM`, `Card`, and `Transaction` define virtual destructors so deleting derived objects through base pointers cleans up properly.

---

## 5. Exception Handling
Error conditions are represented with a custom exception hierarchy rooted at `ATMException` (`src/Exceptions.h`):
- Transactions follow a **validate-first** rule: before changing any account balance or ATM cash, they check limits, funds, bill counts, and input validity, throwing a specific subclass (`InsufficientFundsException`, `InsufficientCashException`, `LimitExceededException`, `InvalidInputException`).
- `Session::runMenuLoop()` catches `const ATMException& e` at the session boundary, prints the localized error message, and ends the session immediately as required by **REQ 9.1**.
- `main.cpp` also wraps file loading and session execution in `try / catch (std::exception&)` so malformed input or unexpected EOF never crashes the program during the live demo.

---

## 6. Standard Template Library (STL)
The project makes practical use of STL containers, utilities, and streams:
- **`std::vector`**: Stores banks, ATMs, per-bank accounts, per-account history, `Session::completedTransactions`, and `TransactionHistory::sessions`.
- **`std::map`**:
  - `CashInventory` uses `std::map<int, int, std::greater<int>>` so denominations stay sorted from `50,000` down to `1,000` for greedy dispensing.
  - `Messages` uses `std::map<std::string, std::string>` for the English and Korean lookup tables.
- **`std::array`**: `Constants::DENOMINATIONS` and `ATMConfig::bills` hold the fixed 4 bill denominations.
- **`std::set`**: Used in `Bank` (registered admin cards) and `SystemLoader` (checking unique 6-digit ATM serial numbers).
- **`std::shared_ptr` (`<memory>`)**: Manages ownership of completed `Transaction` objects stored in `Session`.
- **`std::string`, `std::ifstream`, `std::ofstream`, `std::istringstream`**: Used for parsing `initial_conditions.txt` (`SystemLoader`), reading safe console input (`ConsoleUI`), and exporting transaction history files (`TransactionHistory`).
- **`std::function`**: Used by `ConsoleUI::setSnapshotCallback` to connect the `/` snapshot command without coupling `ConsoleUI` directly to `Snapshot`.
