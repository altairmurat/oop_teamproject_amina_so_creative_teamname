// Owner: Seunggyun (Person A)
#include "Snapshot.h"
#include <utility>
#include "ATM.h"
#include "Bank.h"
#include "CashInventory.h"
#include "Constants.h"

std::vector<const ATM*>& Snapshot::atms() {
    static std::vector<const ATM*> list;
    return list;
}

void Snapshot::setATMs(std::vector<const ATM*> atmList) { atms() = std::move(atmList); }

void Snapshot::print(std::ostream& os) {
    os << "\n==================== SNAPSHOT ====================\n";
    for (const ATM* atm : atms()) {
        const CashInventory& inv = atm->getInventory();
        os << "ATM [SN: " << atm->getSerialNumber() << "] remaining cash: {";
        bool first = true;
        for (int denom : Constants::DENOMINATIONS) {
            if (!first) os << ", ";
            first = false;
            os << "KRW " << denom << " : " << inv.getCount(denom);
        }
        os << "}, total: KRW " << inv.getTotalCash() << "\n";
    }
    for (const Bank* bank : Bank::allBanks())
        for (const Account* acc : bank->getAccounts())
            os << "Account [Bank: " << acc->getBankName() << ", No: " << acc->getAccountNumber()
               << "] Owner: " << acc->getOwnerName() << ", balance: KRW " << acc->getBalance()
               << "\n";
    os << "==================================================\n\n";
}
