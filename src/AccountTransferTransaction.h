#pragma once
#include <string>
#include "Transaction.h"

class AccountTransferTransaction : public Transaction {
public:
    AccountTransferTransaction(ATM& atm, Account& account);

    void execute() override;
    std::string type() const override;
    std::string describe() const override;

private:
    std::string destAccountNumber;
};
