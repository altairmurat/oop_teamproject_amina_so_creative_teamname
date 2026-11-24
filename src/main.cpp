// Owner: Rabia (Person A)
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
#include "Messages.h"
#include "MultiBankATM.h"
#include "Session.h"
#include "SingleBankATM.h"
#include "Snapshot.h"
#include "SystemLoader.h"

using namespace std;

// this is to build the right kind of ATM from the file data.
// we return it as ATM* (upcasting), so both kinds can live in one vector
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

// this is for the ATM list in the main menu, e.g. "000111 (Kakao, Single-Bank, Unilingual)"
string describeATM(const ATMConfig& config) {
    string type = config.isMultiBank ? "Multi-Bank" : "Single-Bank";
    string language = config.isBilingual ? "Bilingual" : "Unilingual";
    return config.serialNumber + " (" + config.primaryBank + ", " + type + ", " + language + ")";
}

// this is the card insertion step. the bank side figures out what card it is (REQ1.9, REQ3.3)
// and then Person B's Session takes over. both card types are passed as const Card& (upcasting)
// and Session calls the virtual isAdmin() to know where to go
void runOneSession(ATM& atm) {
    // language isn't chosen yet at this point (Session asks after the card), so English here
    string cardNumber = ConsoleUI::readLine(Messages::get("insert_card", Language::English));

    if (Bank::isAdminCard(cardNumber)) {
        // admin card has no issuing bank, so we just hand the ATM's primary bank to the Session
        Bank* primaryBank = Bank::findBank(atm.getPrimaryBankName());
        if (primaryBank == nullptr) {
            ConsoleUI::showMessage(Messages::get("invalid_card", Language::English));
            return;
        }
        AdminCard adminCard(cardNumber);
        Session session(atm);
        session.run(adminCard, *primaryBank);
        return;
    }

    // normal card: ask all banks who issued it
    Account* account = Bank::findAccountByCard(cardNumber);
    if (account == nullptr) {
        ConsoleUI::showMessage(Messages::get("invalid_card", Language::English));
        return;
    }
    Bank* issuingBank = Bank::findBank(account->getBankName());
    Card card(cardNumber, account->getBankName());
    Session session(atm); // Session checks if THIS ATM accepts the card (single vs multi bank)
    session.run(card, *issuingBank);
}

// this is to free everything we made with new. ATM has a virtual destructor,
// so deleting through ATM* also runs the SingleBankATM/MultiBankATM destructor
void deleteAll(vector<ATM*>& atms, vector<Bank*>& banks) {
    for (size_t i = 0; i < atms.size(); i++) {
        delete atms[i];
    }
    atms.clear();
    for (size_t i = 0; i < banks.size(); i++) {
        delete banks[i]; // the bank deletes its own accounts
    }
    banks.clear();
}

int main(int argc, char* argv[]) {
    // the file can be given as an argument, otherwise we use the default one
    string path = "data/initial_conditions.txt";
    if (argc > 1) {
        path = argv[1];
    }

    vector<Bank*> banks;
    vector<ATMConfig> atmConfigs;
    vector<ATM*> atms;

    // 1. load everything from the file (REQ1.11), nothing is hard-coded
    try {
        SystemLoader::load(path, banks, atmConfigs);
        for (size_t i = 0; i < atmConfigs.size(); i++) {
            atms.push_back(createATM(atmConfigs[i]));
        }
    } catch (exception& e) {
        cerr << "Failed to load initial conditions: " << e.what() << "\n";
        deleteAll(atms, banks); // clean up whatever was already created
        return 1;
    }

    // 2. hook up the snapshot so "/" works at any prompt (REQ10.1)
    Snapshot::setATMs(atms);
    ConsoleUI::setSnapshotCallback(&Snapshot::print);

    // 3. main loop: pick an ATM, run one session on it, repeat until 0
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
                break; // input is over (for example a test file piped into the program)
            }
            // this is the last safety net: even if something unexpected happens, the demo must not crash
            cout << "Session ended: " << e.what() << "\n";
        }
    }

    deleteAll(atms, banks);
    cout << "Goodbye.\n";
    return 0;
}
