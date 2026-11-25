// Owner: Amina
#pragma once
#include <string>
#include "Constants.h"

class ATM;
class Account;
class Bank;

// Abstract base class of every transaction (deposit, withdrawal, cash transfer, account transfer).
// Rules for subclasses:
//   * execute() asks the user for input, validates EVERYTHING first, and only then changes
//     balances/cash. On any problem it throws an ATMException and changes nothing.
//   * The last statement of a successful execute() must be commit(), which gives the
//     transaction its unique ID. Failed transactions never get an ID.

class Transaction {
public:
    Transaction(ATM& atm, Account& account);
    virtual ~Transaction() {}

    virtual void execute() = 0; // throws ATMException on failure
    virtual std::string type() const = 0; // e.g. "Cash Deposit"
    virtual std::string describe() const = 0; // one history line

    int transactionID() const { return id; } // 0 until transaction succeded
    long long transactionAmount() const { return amount; } // money moved w/out the fee
    long long transactionFee() const { return fee; }
    const ATM& atm() const { return transactionATM; }
    const Account& account() const { return transactionAccount; }

protected:
    void commit(); // assigns a unique ID
    std::string tr(const std::string& key) const; // messages in the ATM'S current language
    Bank& issuingBank() const; // the bank that owns transactionAccount
    std::string money(long long krw) const; // "KRW 4,000" (EN) or "4,000원" (KR)

    ATM& transactionATM;
    Account& transactionAccount;
    long long amount = 0;
    long long fee = 0;

private:
    int id = 0;
    static int nextID;
};