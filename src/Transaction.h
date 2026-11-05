// Owner: Amina
#pragma once
#include <string>
#include "Constants.h"
using namespace std;

class ATM;
class Account;

// Abstract base class of every transaction (deposit, withdrawal, cash transfer, account transfer).
// Rules for subclasses:
//   * execute() asks the user for input, validates EVERYTHING first, and only then changes
//     balances/cash. On any problem it throws an ATMException and changes nothing.
//   * The last statement of a successful execute() must be commit(), which gives the
//     transaction its unique ID. Failed transactions never get an ID.

class Transaction {
public:
    Transaction(ATM& atm, Account& account);
    virtual ~Transaction() = 0;

    virtual void execute() = 0; // throws ATMException on failure
    virtual string type() const = 0; // e.g. "Cash Deposit"
    virtual string describe() const = 0; // one history line

    int transactionID() const { return id; } // 0 until transaction succeded
    long long transactionAmount() const { return amount; } // money moved w/out the fee
    long long transactionFee() const { return fee; }
    const ATM& atm() const { return transactionATM; }
    const Account& account() const { return transactionAccount; }

protected:
    void commit(); // assigns a unique ID
    string tr(const string& key) const; // messages in the ATM'S current language
    BillCount readBills() const; // asks for a count per denomination, max 50 bills

    // АЛЬТАИР сделай Constants.h пж, BillCount твой

    ATM& transactionATM;
    Account& transactionAccount;
    long long amount = 0;
    long long fee = 0;

private:
    int id = 0;
    static int nextID;
};