// Owner: Amina
#pragma once
#include "Transaction.h"

class WithdrawalTransaction : public Transaction {
public:
    // priorWithdrawals = num of successful withdrawals done in this session
    // (Session::getWithdrawalCount()). Session calls recordWithdrawal() after execute() succeeds
    WithdrawalTransaction(ATM& atm, Account& account, int withdrawalsSoFar);
    void execute() override;
    std::string type() const override { return "Cash Withdrawal"; }
    std::string describe() const override;

private:
    int priorWithdrawals;
};