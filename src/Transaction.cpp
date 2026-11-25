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

// this is to print amounts the same way everywhere: 4000 -> "KRW 4,000" / "4,000원"
std::string Transaction::money(long long krw) const {
    std::string digits = std::to_string(krw < 0 ? -krw : krw);
    std::string grouped;
    for (size_t i = 0; i < digits.size(); ++i) {
        if (i > 0 && (digits.size() - i) % 3 == 0) grouped += ',';
        grouped += digits[i];
    }
    if (krw < 0) grouped = "-" + grouped;
    if (transactionATM.getLanguage() == Language::Korean) return grouped + tr("tx.won_suffix");
    return tr("tx.krw_prefix") + grouped;
}

Bank& Transaction::issuingBank() const {
    Bank* bank = Bank::findBank(transactionAccount.getBankName());
    if (!bank) throw ATMException("Bank not found: " + transactionAccount.getBankName());
    return *bank;
}