// Owner: Amina
#include "WithdrawalTransaction.h"
#include <string>
#include "ATM.h"
#include "Account.h"
#include "Bank.h"
#include "ConsoleUI.h"
#include "Constants.h"
#include "Exceptions.h"
#include "FeePolicy.h"

WithdrawalTransaction::WithdrawalTransaction(ATM& atm, Account& account, int withdrawalsSoFar)
    : Transaction(atm, account), priorWithdrawals(withdrawalsSoFar) {}

void WithdrawalTransaction::execute() {
    // REQ5.6: at most 3 withdrawals per session
    if (priorWithdrawals >= Constants::MAX_WITHDRAWALS_PER_SESSION) throw LimitExceededException(tr("err.withdraw_count"));

    const bool primary = transactionATM.isPrimaryBank(transactionAccount.getBankName());
    const long long requiredFee = FeePolicy::withdrawalFee(primary);

    // REQ5.1: the user types only the amount, never bill counts
    const long long requested = ConsoleUI::readLongLong(tr("withdraw.enter_amount"), 1, 100000000LL);

    // validate b4 touching any state
    if (requested > Constants::MAX_WITHDRAWAL_AMOUNT) throw LimitExceededException(tr("err.withdraw_max")); // REQ5.7
    if (requested % Constants::DENOM_1000 != 0) throw InvalidInputException(tr("err.amount_unit"));
    if (!transactionAccount.canWithdraw(requested + requiredFee)) throw InsufficientFundsException(tr("err.no_funds")); // REQ5.2: amount + fee

    // CashInventory works out the fewest bills & removes them from the ATM in one step
    BillCount plan;
    if (!transactionATM.getInventory().tryDispense(requested, plan)) throw InsufficientCashException(tr("err.no_cash_atm")); // REQ5.2: ATM cash
    if (plan.totalBills() > Constants::MAX_BILLS_PER_DEPOSIT) { // the 50-bill limit also applies to dispensing
        transactionATM.addCash(plan); // put the bills back, nothing happened
        throw LimitExceededException(tr("err.too_many_dispense"));
    }
    
    // everything is valid -> debit the account (REQ5.3 + REQ5.4: amount and fee leave the account
    issuingBank().withdraw(transactionAccount, requested + requiredFee);
    amount = requested;
    fee = requiredFee;

    ConsoleUI::showMessage(tr("withdraw.dispensing"));
    for (int denom : Constants::DENOMINATIONS)
        if (plan.get(denom) > 0)
            ConsoleUI::showMessage(" KRW " + std::to_string(denom) + " x " + std::to_string(plan.get(denom)));
    ConsoleUI::showMessage(tr("withdraw.fee_label") + std::to_string(fee));
    ConsoleUI::showMessage(tr("withdraw.success"));
    commit();
}

std::string WithdrawalTransaction::describe() const {
    return money(amount) + tr("tx.withdrawn_from") + transactionAccount.getAccountNumber();
}