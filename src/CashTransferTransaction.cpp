#include "CashTransferTransaction.h"
#include "ATM.h"
#include "Account.h"
#include "Bank.h"
#include "CashInventory.h"
#include "ConsoleUI.h"
#include "Exceptions.h"
#include "FeePolicy.h"
#include <iostream>

using namespace std;

CashTransferTransaction::CashTransferTransaction(ATM& atm, Account& account)
    : Transaction(atm, account) {}

void CashTransferTransaction::execute() {
    string destNum = ConsoleUI::readLine(tr("enter_dest_account"));
    Account* dest = Bank::findAccount(destNum);
    if (dest == nullptr) {
        throw InvalidInputException(tr("account_not_found"));
    }

    cout << tr("insert_cash_bills") << "\n";
    BillCount inserted = readBills();
    long long totalInserted = inserted.total();

    // fee is always 1000 won and it is deducted from inserted cash, инвентарь обновляется ниже
    long long transferFee = FeePolicy::cashTransferFee();
    if (totalInserted <= transferFee) {
        throw InvalidInputException(tr("invalid_input"));
    }

    amount = totalInserted - transferFee;
    fee = transferFee;
    destAccountNumber = dest->getAccountNumber();

    dest->deposit(amount);
    transactionATM.getInventory().addBills(inserted);

    cout << tr("fee_notice") << fee << "\n";
    cout << tr("transfer_success") << "\n";

    commit();
}

string CashTransferTransaction::type() const {
    return "Cash Transfer";
}

string CashTransferTransaction::describe() const {
    return "Cash Transfer: -> " + destAccountNumber
        + " | Amount: " + to_string(amount) + " KRW"
        + " | Fee: " + to_string(fee) + " KRW";
}
