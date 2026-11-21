// Owner: Amina
#include "Transaction.h"
#include "ATM.h"
#include "Account.h"
#include "Bank.h"
#include "Exceptions.h"
#include "Messages.h"

int Transaction::nextID = 0;

Transaction::Transaction(ATM& atm, Account& account): transactionATM(atm), transactionAccount(account) {}

void Transaction::commit() { id = ++nextID; }

std::string Transaction::tr(const std::string& key) const {
    return Messages::get(key, transactionATM.getLanguage());
}

Bank& Transaction::issuingBank() const {
    Bank* bank = Bank::findBank(transactionAccount.getBankName());
    if (!bank) throw ATMException("Bank not found: " + transactionAccount.getBankName());
    return *bank;
}