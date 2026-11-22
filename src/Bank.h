// Owner: Seunggyun (Person A)
#pragma once
#include <string>
#include <vector>
#include "Account.h"

// A bank owns its accounts (REQ1.5, REQ1.6). The accounts are created with new
// and deleted in the destructor.
//
// Every Bank also adds itself to the static list m_allBanks, so that static
// functions such as findAccount() can search all banks without anyone passing
// a list of banks around (one copy shared by all objects, Lecture 9).
//
// Banks must be created with new (SystemLoader does this) and deleted by main().
class Bank {
private:
    std::string m_name;
    std::vector<Account*> m_accounts;

    static std::vector<Bank*> m_allBanks;
    static std::vector<std::string> m_adminCards; // stored without dashes

public:
    Bank(const std::string& name);
    ~Bank();

    // A copy would share the same Account pointers and both copies would
    // delete them (the shallow copy problem from Lecture 9), so copying is forbidden.
    Bank(const Bank&) = delete;
    Bank& operator=(const Bank&) = delete;

    std::string getName() const { return m_name; }
    std::vector<Account*> getAccounts() const { return m_accounts; }

    // Creates a new account in this bank. Throws InvalidInputException
    // if the account number already exists in any bank.
    void openAccount(const std::string& ownerName, const std::string& accountNumber,
                     long long balance, const std::string& cardNumber,
                     const std::string& password);

    bool hasAccount(const Account& account) const;

    // REQ3.3 (bank side): the ATM asks the bank, the bank answers.
    bool verifyCard(const std::string& cardNumber) const;
    bool verifyPassword(const std::string& cardNumber, const std::string& password) const;
    Account* getAccountByCard(const std::string& cardNumber) const; // nullptr if not found

    // Balance changes for an account of this bank.
    void deposit(Account& account, long long amount);
    void withdraw(Account& account, long long amount);

    // ---- searches over all banks ----
    static Account* findAccount(const std::string& accountNumber);   // nullptr if not found
    static Account* findAccountByCard(const std::string& cardNumber); // nullptr if not found
    static Bank* findBank(const std::string& name);                   // nullptr if not found
    static std::vector<Bank*> getAllBanks();

    // Moves amount from source to destination and charges fee to source (REQ6.x).
    // Checks everything first, so on an exception no balance has changed.
    static void transfer(Account& source, Account& destination, long long amount, long long fee);

    // ---- admin cards (REQ1.9) ----
    static void addAdminCard(const std::string& cardNumber);
    static bool isAdminCard(const std::string& cardNumber);
};
