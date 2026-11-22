// Owner: Seunggyun (Person A)
#include "Snapshot.h"
#include <iostream>
#include "ATM.h"
#include "Bank.h"
#include "CashInventory.h"
#include "Constants.h"

using namespace std;

vector<ATM*> Snapshot::m_atms;

void Snapshot::setATMs(const vector<ATM*>& atms) {
    m_atms = atms;
}

void Snapshot::print() {
    cout << "\n==================== SNAPSHOT ====================\n";

    for (size_t i = 0; i < m_atms.size(); i++) {
        const CashInventory& inventory = m_atms[i]->getInventory();
        cout << "ATM [SN: " << m_atms[i]->getSerialNumber() << "] remaining cash: {";
        for (size_t d = 0; d < Constants::DENOMINATIONS.size(); d++) {
            int denomination = Constants::DENOMINATIONS[d];
            if (d > 0) {
                cout << ", ";
            }
            cout << "KRW " << denomination << " : " << inventory.getCount(denomination);
        }
        cout << "}, total: KRW " << inventory.getTotalCash() << "\n";
    }

    vector<Bank*> banks = Bank::getAllBanks();
    for (size_t b = 0; b < banks.size(); b++) {
        vector<Account*> accounts = banks[b]->getAccounts();
        for (size_t a = 0; a < accounts.size(); a++) {
            cout << "Account [Bank: " << accounts[a]->getBankName()
                 << ", No: " << accounts[a]->getAccountNumber()
                 << "] Owner: " << accounts[a]->getOwnerName()
                 << ", balance: KRW " << accounts[a]->getBalance() << "\n";
        }
    }

    cout << "==================================================\n\n";
}
