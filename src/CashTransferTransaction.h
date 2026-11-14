#pragma once
#include "Transaction.h"
#include <string>

class CashTransferTransaction : public Transaction {
public:
    CashTransferTransaction(ATM& atm, Account& account);

    void execute() override;
    std::string type() const override;
    std::string describe() const override;

private:
    std::string destAccountNumber;
};
