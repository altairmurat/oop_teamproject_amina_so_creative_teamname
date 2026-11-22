#include "AccountTransferTransaction.h"
#include <iostream>
#include "ATM.h"
#include "Bank.h"
#include "Account.h"
#include "FeePolicy.h"
#include "ConsoleUI.h"
#include "Exceptions.h"

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

    long long money = ConsoleUI::readPositiveAmount(tr("enter_amount"));

    // комиссию беру из FeePolicy, названия методов не меняйте пж или скажите если поменяете
    // fee is calculated based on if source/dest bank is matched with primary bank
    bool srcPrimary = transactionATM.isPrimaryBank(transactionAccount.getBankName());
    bool dstPrimary = transactionATM.isPrimaryBank(dest->getBankName());
    long long f = FeePolicy::accountTransferFee(srcPrimary, dstPrimary);

    // balance must be checked before anything is changed
    if (!transactionAccount.canWithdraw(money + f)) {
        throw InsufficientFundsException(tr("insufficient_funds"));
    }

    amount = money;
    fee = f;
    destAccountNumber = dest->getAccountNumber();

    Bank::transfer(transactionAccount, *dest, amount, fee);

    ConsoleUI::showMessage(tr("fee_notice") + to_string(fee));
    ConsoleUI::showMessage(tr("transfer_success"));

    commit();
}

string AccountTransferTransaction::type() const {
    return "Account Transfer";
}

string AccountTransferTransaction::describe() const {
    return "Account Transfer " + transactionAccount.getAccountNumber()
        + " -> " + destAccountNumber + " (" + to_string(amount) + " Won)";
}
