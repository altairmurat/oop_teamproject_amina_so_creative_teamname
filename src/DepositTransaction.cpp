// Owner: Amina
#include "DepositTransaction.h"
#include <string>
#include "ATM.h"
#include "Account.h"
#include "Bank.h"
#include "ConsoleUI.h"
#include "Constants.h"
#include "Exceptions.h"
#include "FeePolicy.h"

void DepositTransaction::execute() {
    const bool primary = transactionATM.isPrimaryBank(transactionAccount.getBankName());
    const int requiredFee = FeePolicy::depositFee(primary);

    // 1) cash/check?
    isCheck = (ConsoleUI::readInt(tr("deposit.choose_type"), 1, 2) == 2);

    // 2) the deposit
    BillCount cash; //all 0s
    long long depositAmount = 0;
    if (isCheck) {
        depositAmount = ConsoleUI::readLongLong(tr("deposit.enter_check"), 1, 1000000000LL);
        if (depositAmount < Constants::MIN_CHECK_AMOUNT) throw InvalidInputException(tr("err.check_too_small"));
    } else {
        cash = ConsoleUI::readBills(Constants::MAX_BILLS_PER_DEPOSIT); // // enforces the 50-bill limit (REQ4.2)
        depositAmount = cash.total();
        if (depositAmount == 0) throw InvalidInputException(tr("err.no_cash"));
    }

    // 3) the fee is entered separately and always as cash (REQ4.4)
    BillCount feeBills;
    if (requiredFee > 0) {
        ConsoleUI::showMessage(tr("deposit.fee_label") + std::to_string(requiredFee));
        feeBills = ConsoleUI::readBills(Constants::MAX_BILLS_PER_DEPOSIT);
        if (feeBills.total() != requiredFee) throw InvalidInputException(tr("err.fee_mismatch"));
    }

    // 4) everything is valid -> change the state
    issuingBank().deposit(transactionAccount, depositAmount); // REQ4.3: the bank account gets the money
    if (!isCheck) transactionATM.addCash(cash); // REQ4.5 cash raises ATM cash, REQ4.6 checks do not
    transactionATM.addCash(feeBills); // the fee cash is physically inside the ATM too
    amount = depositAmount;
    fee = requiredFee;
    ConsoleUI::showMessage(tr("deposit.success"));
    commit();
}

std::string DepositTransaction::describe() const {
    return money(amount) + tr("tx.added_to") + transactionAccount.getAccountNumber();
}