// Owner: Amina
#pragma once
#include "Transaction.h"

class DepositTransaction : public Transaction {
public:
    using Transaction::Transaction; // (ATM&, Account&)
    void execute() override;
    std::string type() const override { return tr(isCheck ? "tx.check_deposit" : "tx.cash_deposit"); }
    std::string describe() const override;

private:
    bool isCheck = false;
};