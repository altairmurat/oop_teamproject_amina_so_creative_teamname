// Owner: Altair (Person B)
#pragma once
#include <memory>
#include <string>
#include <vector>
#include "Constants.h"

class ATM;
class Bank;
class Card;
class Account;
class Transaction;

// this is to manage one user session from card insert to card return
class Session {
public:
    explicit Session(ATM& atm);

    void run(const Card& card, Bank& issuingBank);

    bool isActive() const { return active; }
    bool isAuthorized() const { return authorized; }
    bool canWithdraw() const;
    void recordWithdrawal();

    int getWrongPasswordAttempts() const { return wrongPasswordAttempts; }
    int getWithdrawalCount() const { return withdrawalCount; }
    const std::vector<std::shared_ptr<Transaction>>& getCompletedTransactions() const {
        return completedTransactions;
    }

    void addCompletedTransaction(const std::shared_ptr<Transaction>& tx);
    void printSummary() const;
    void endSession();

private:
    ATM& atm;
    bool active = false;
    bool authorized = false;
    int wrongPasswordAttempts = 0;
    int withdrawalCount = 0;
    std::vector<std::shared_ptr<Transaction>> completedTransactions;
    std::string summaryCard;     // card number shown in the summary (REQ2.3)
    std::string summaryAccount;  // account number shown in the summary (REQ2.3)

    bool verifyCard(const Card& card);
    bool authorizeUser(const Card& card, Bank& issuingBank);
    void runMenuLoop(Account& account, const Card& card);
};
