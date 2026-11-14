#include "AccountTransferTransaction.h"
#include "ATM.h"
#include "Account.h"
#include "Bank.h"
#include "ConsoleUI.h"
#include "Exceptions.h"
#include "FeePolicy.h"
#include <iostream>

using namespace std;

AccountTransferTransaction::AccountTransferTransaction(ATM& atm, Account& account)
    : Transaction(atm, account) {}

void AccountTransferTransaction::execute() {
    string destNum = ConsoleUI::readLine(tr("enter_dest_account"));
    Account* dest = Bank::findAccount(destNum);
    if(dest == nullptr) {
        throw InvalidInputException(tr("account_not_found"));
    }
    if (dest == &transactionAccount) {
        throw InvalidInputException(tr("same_account"));
    }

    long long transferAmount = ConsoleUI::readPositiveAmount(tr("enter_amount"));

    // комиссию беру из FeePolicy, названия методов не меняйте пж или скажите если поменяете
    // fee is calculated based on if source/dest bank is matched with primary bank
    string primaryBank = transactionATM.getPrimaryBankName();
    bool srcPrimary = (transactionAccount.getBankName() == primaryBank);
    bool destPrimary = (dest->getBankName() == primaryBank);
    long long transferFee = FeePolicy::accountTransferFee(srcPrimary, destPrimary);

    // balance must be checked before anything is changed
    if (transactionAccount.getBalance() < transferAmount + transferFee) {
        throw InsufficientFundsException(tr("insufficient_funds"));
    }

    amount = transferAmount;
    fee = transferFee;
    destAccountNumber = dest->getAccountNumber();

    Bank::transfer(transactionAccount, *dest, amount, fee);

    cout << tr("fee_notice") << fee << "\n";
    cout << tr("transfer_success") << "\n";

    commit();
}

string AccountTransferTransaction::type() const {
    return "Account Transfer";
}

string AccountTransferTransaction::describe() const {
    return "Account Transfer: " + transactionAccount.getAccountNumber()
        + " -> " + destAccountNumber
        + " | Amount: " + to_string(amount) + " KRW"
        + " | Fee: " + to_string(fee) + " KRW";
}
