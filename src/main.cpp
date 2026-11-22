// Owner: Seunggyun (Person A)
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "ATM.h"
#include "Bank.h"
#include "Card.h"
#include "CashInventory.h"
#include "ConsoleUI.h"
#include "Constants.h"
#include "MultiBankATM.h"
#include "Session.h"
#include "SingleBankATM.h"
#include "Snapshot.h"
#include "SystemLoader.h"

using namespace std;

// Creates the right kind of ATM. The result is returned as an ATM* (upcasting),
// so main() can keep both kinds of ATM in one vector.
ATM* createATM(const ATMConfig& config) {
    LanguageMode mode = LanguageMode::Unilingual;
    if (config.isBilingual) {
        mode = LanguageMode::Bilingual;
    }
    CashInventory cash(config.bills[0], config.bills[1], config.bills[2], config.bills[3]);

    if (config.isMultiBank) {
        return new MultiBankATM(config.serialNumber, config.primaryBank, mode, cash);
    }
    return new SingleBankATM(config.serialNumber, config.primaryBank, mode, cash);
}

string describeATM(const ATMConfig& config) {
    string type = config.isMultiBank ? "Multi-Bank" : "Single-Bank";
    string language = config.isBilingual ? "Bilingual" : "Unilingual";
    return config.serialNumber + " (" + config.primaryBank + ", " + type + ", " + language + ")";
}

// Card insertion. The bank side decides what kind of card it is (REQ1.9, REQ3.3),
// then the Session (Person B) takes over. Both card types are passed as
// const Card& (upcasting), and Session calls the virtual isAdmin().
void runOneSession(ATM& atm) {
    string cardNumber = ConsoleUI::readLine("Insert card (enter card number): ");

    if (Bank::isAdminCard(cardNumber)) {
        // An admin card has no issuing bank, so the ATM's primary bank is used.
        Bank* primaryBank = Bank::findBank(atm.getPrimaryBankName());
        if (primaryBank == nullptr) {
            ConsoleUI::showMessage("Invalid card. The card is returned.");
            return;
        }
        AdminCard adminCard(cardNumber);
        Session session(atm);
        session.run(adminCard, *primaryBank);
        return;
    }

    Account* account = Bank::findAccountByCard(cardNumber);
    if (account == nullptr) {
        ConsoleUI::showMessage("Invalid card. The card is returned.");
        return;
    }
    Bank* issuingBank = Bank::findBank(account->getBankName());
    Card card(cardNumber, account->getBankName());
    Session session(atm);
    session.run(card, *issuingBank);
}

// Frees everything created with new. ATM has a virtual destructor,
// so deleting through ATM* also runs the derived class destructor.
void deleteAll(vector<ATM*>& atms, vector<Bank*>& banks) {
    for (size_t i = 0; i < atms.size(); i++) {
        delete atms[i];
    }
    atms.clear();
    for (size_t i = 0; i < banks.size(); i++) {
        delete banks[i]; // also deletes the bank's accounts
    }
    banks.clear();
}

int main(int argc, char* argv[]) {
    string path = "data/initial_conditions.txt";
    if (argc > 1) {
        path = argv[1];
    }

    vector<Bank*> banks;
    vector<ATMConfig> atmConfigs;
    vector<ATM*> atms;

    // 1. Load everything from the file (REQ1.11). Nothing is hard-coded.
    try {
        SystemLoader::load(path, banks, atmConfigs);
        for (size_t i = 0; i < atmConfigs.size(); i++) {
            atms.push_back(createATM(atmConfigs[i]));
        }
    } catch (exception& e) {
        cerr << "Failed to load initial conditions: " << e.what() << "\n";
        deleteAll(atms, banks);
        return 1;
    }

    // 2. "/" at any prompt prints the snapshot (REQ10.1).
    Snapshot::setATMs(atms);
    ConsoleUI::setSnapshotCallback(&Snapshot::print);

    // 3. Main loop: choose an ATM, run one session on it, repeat.
    while (true) {
        cout << "\n===== ATM SYSTEM =====\n";
        for (size_t i = 0; i < atmConfigs.size(); i++) {
            cout << "  " << (i + 1) << ". ATM " << describeATM(atmConfigs[i]) << "\n";
        }
        cout << "  0. Exit   (type / at any prompt for a snapshot)\n";

        try {
            int choice = ConsoleUI::readMenuChoice("Select an ATM: ", 0, static_cast<int>(atms.size()));
            if (choice == 0) {
                break;
            }
            runOneSession(*atms[choice - 1]);
        } catch (exception& e) {
            if (!cin) {
                break; // input ended, e.g. a piped test file
            }
            // Last line of defense: a bug must not crash the demo.
            cout << "Session ended: " << e.what() << "\n";
        }
    }

    deleteAll(atms, banks);
    cout << "Goodbye.\n";
    return 0;
}
