// Owner: Seunggyun (Person A)
#pragma once
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "Account.h"
#include "Card.h"

// A bank owns its accounts (REQ1.5, REQ1.6). Every Bank registers itself in a
// static registry so that findAccount() etc. can search across all banks
// without anyone passing a bank list around.
class Bank {
public:
    explicit Bank(std::string name);
    ~Bank();
    Bank(const Bank&) = delete;
    Bank& operator=(const Bank&) = delete;

    const std::string& getName() const { return name; }

    // Creates an account in this bank. Throws std::invalid_argument if the
    // account number already exists anywhere in the system.
    Account& openAccount(const std::string& owner, const std::string& accountNumber,
                         long long balance, const std::string& cardNumber,
                         const std::string& password);

    std::vector<Account*> getAccounts() const;
    bool owns(const Account& acc) const;

    // REQ3.3 (bank side): the ATM asks the bank, the bank answers.
    bool verifyCard(const std::string& cardNumber) const;          // issued by this bank?
    bool verifyPassword(const std::string& cardNumber, const std::string& pw) const;
    Account* getAccountByCard(const std::string& cardNumber) const; // in this bank only

    // Balance changes for an account of this bank (throws if it is not ours).
    void deposit(Account& acc, long long amount);
    void withdraw(Account& acc, long long amount);

    // ---- system-wide lookups (all banks) ----
    static Account* findAccount(const std::string& accountNumber);  // dash-insensitive
    static Account* findAccountByCard(const std::string& cardNumber);
    static Bank* findBank(const std::string& name);
    static const std::vector<Bank*>& allBanks();

    // Moves amount from src to dst and charges fee to src (REQ6.x).
    // Caller validates first; throws only as a safety net.
    static void transfer(Account& src, Account& dst, long long amount, long long fee);

    // ---- cards ----
    static void registerAdminCard(const std::string& cardNumber);
    static bool isAdminCard(const std::string& cardNumber);
    // Returns AdminCard / Card for a known number, nullptr for an unknown one.
    static std::unique_ptr<Card> lookupCard(const std::string& cardNumber);

private:
    std::string name;
    std::vector<std::unique_ptr<Account>> accounts;

    static std::vector<Bank*>& registry();
    static std::set<std::string>& adminCards();
};
