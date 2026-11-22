// Owner: Amina
#pragma once
#include "Transaction.h"

class DepositTransaction : public Transaction {
public:
    using Transaction::Transaction; // (ATM&, Account&)
    void execute() override;
    std::string type() const override { return isCheck ? "Check Deposit" : "Cash Deposit"; }
    std::string describe() const override;

private:
    bool isCheck = false;
};