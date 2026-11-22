// Owner: Alan (Person D)
#include "CashTransferTransaction.h"
#include <iostream>
#include "ATM.h"
#include "Bank.h"
#include "Account.h"
#include "FeePolicy.h"
#include "ConsoleUI.h"
#include "Constants.h"
#include "Exceptions.h"

using namespace std;

CashTransferTransaction::CashTransferTransaction(ATM& atm, Account& account)
    : Transaction(atm, account) {}

void CashTransferTransaction::execute() {
    string destNum = ConsoleUI::readLine(tr("enter_dest_account"));
    Account* dest = Bank::findAccount(destNum);
    if(dest == nullptr) {
        throw InvalidInputException(tr("account_not_found"));
    }

    ConsoleUI::showMessage(tr("insert_cash_bills"));
    BillCount bills = ConsoleUI::readBills(Constants::MAX_BILLS_PER_DEPOSIT);
    long long total = bills.total();

    // 1000 KRW fee is deducted from the inserted cash
    long long f = FeePolicy::cashTransferFee();
    if (total <= f) {
        throw InvalidInputException(tr("cash_transfer_too_small"));
    }

    Bank* destBank = Bank::findBank(dest->getBankName());
    if (destBank == nullptr) {
        throw ATMException(tr("bank_not_found"));
    }

    amount = total - f;
    fee = f;
    destAccountNumber = dest->getAccountNumber();

    // deposit transfer amount to destination bank and add physical bills to ATM
    destBank->deposit(*dest, amount);
    transactionATM.addCash(bills);

    ConsoleUI::showMessage(tr("fee_notice") + to_string(fee));
    ConsoleUI::showMessage(tr("transfer_success"));

    commit();
}

string CashTransferTransaction::type() const {
    return "Cash Transfer";
}

string CashTransferTransaction::describe() const {
    return "Cash Transfer -> " + destAccountNumber + " (" + to_string(amount) + " Won)";
}
