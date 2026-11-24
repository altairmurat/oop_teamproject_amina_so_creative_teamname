// Owner: Seunggyun (Person A)
#pragma once
#include <string>
#include <vector>
#include "Account.h"

// this is a bank: it owns its accounts (REQ1.5, REQ1.6).
// accounts are made with new in openAccount() and deleted in the destructor.
//
// every Bank also puts itself into the static list m_allBanks, so static functions
// like findAccount() can search ALL banks without anyone passing a list around
// (static = one copy shared by every Bank object).
//
// banks are created with new by SystemLoader and deleted by main()
class Bank {
private:
    std::string m_name;
    std::vector<Account*> m_accounts;

    static std::vector<Bank*> m_allBanks;
    static std::vector<std::string> m_adminCards; // saved without dashes

public:
    Bank(const std::string& name);
    ~Bank();

    // copying is forbidden: a copy would hold the same Account pointers and
    // both copies would delete them (shallow copy problem)
    Bank(const Bank&) = delete;
    Bank& operator=(const Bank&) = delete;

    std::string getName() const { return m_name; }
    std::vector<Account*> getAccounts() const { return m_accounts; }

    // this is to make a new account inside this bank. one user can have many accounts.
    // throws InvalidInputException if the account number or card number is already used
    void openAccount(const std::string& ownerName, const std::string& accountNumber,
                     long long balance, const std::string& cardNumber,
                     const std::string& password);

    bool hasAccount(const Account& account) const;

    // REQ3.3 (bank side): the ATM doesn't know passwords, it asks the bank and the bank answers
    bool verifyCard(const std::string& cardNumber) const;
    bool verifyPassword(const std::string& cardNumber, const std::string& password) const;
    Account* getAccountByCard(const std::string& cardNumber) const; // nullptr if not found

    // balance changes for an account that belongs to this bank
    void deposit(Account& account, long long amount);
    void withdraw(Account& account, long long amount);

    // ---- searches over all banks ----
    static Account* findAccount(const std::string& accountNumber);   // nullptr if not found
    static Account* findAccountByCard(const std::string& cardNumber); // nullptr if not found
    static Bank* findBank(const std::string& name);                   // nullptr if not found
    static std::vector<Bank*> getAllBanks();

    // this is to move money between two accounts (can be different banks) and charge the fee
    // to the source (REQ6.x). everything is checked first, so if it throws nothing has changed
    static void transfer(Account& source, Account& destination, long long amount, long long fee);

    // ---- admin cards (REQ1.9) ----
    static void addAdminCard(const std::string& cardNumber);
    static bool isAdminCard(const std::string& cardNumber);
    static void clearAdminCards(); // this is to start fresh every time the file is loaded
};
