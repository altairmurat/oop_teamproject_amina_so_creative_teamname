// Owner: Seunggyun (Person A)
#include <exception>
#include <iostream>
#include <memory>
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

namespace {

// The only place that calls Person B's constructors.
std::unique_ptr<ATM> makeATM(const ATMConfig& cfg) {
    LanguageMode mode = cfg.bilingual ? LanguageMode::Bilingual : LanguageMode::Unilingual;
    CashInventory cash(cfg.bills[0], cfg.bills[1], cfg.bills[2], cfg.bills[3]); // 50k,10k,5k,1k
    if (cfg.multiBank)
        return std::make_unique<MultiBankATM>(cfg.serialNumber, cfg.primaryBank, mode, cash);
    return std::make_unique<SingleBankATM>(cfg.serialNumber, cfg.primaryBank, mode, cash);
}

std::string describe(const ATMConfig& cfg) {
    return cfg.serialNumber + " (" + cfg.primaryBank + ", " +
           (cfg.multiBank ? "Multi-Bank" : "Single-Bank") + ", " +
           (cfg.bilingual ? "Bilingual" : "Unilingual") + ")";
}

// Card insertion: the bank side decides what the card is (REQ1.9, REQ3.3).
void runOneSession(ATM& atm) {
    std::string number = ConsoleUI::readLine("Insert card (enter card number): ");
    std::unique_ptr<Card> card = Bank::lookupCard(number);
    if (!card) {
        ConsoleUI::showMessage("Invalid card. The card is returned.");
        return;
    }
    // An admin card has no issuing bank; the ATM's primary bank stands in.
    Bank* issuing = Bank::findBank(card->isAdmin() ? atm.getPrimaryBankName()
                                                   : card->getBankName());
    if (!issuing) {
        ConsoleUI::showMessage("Invalid card. The card is returned.");
        return;
    }
    Session session(atm);
    session.run(*card, *issuing);
}

} // namespace

int main(int argc, char* argv[]) {
    const std::string path = (argc > 1) ? argv[1] : "data/initial_conditions.txt";

    // 1. Load everything from the file (REQ1.11). No hard-coded data.
    SystemData system;
    std::vector<std::unique_ptr<ATM>> atms;
    try {
        system = SystemLoader::load(path);
        for (const ATMConfig& cfg : system.atms) atms.push_back(makeATM(cfg));
    } catch (const std::exception& e) {
        std::cerr << "Failed to load initial conditions: " << e.what() << "\n";
        return 1;
    }

    // 2. "/" at any prompt prints the snapshot (REQ10.1).
    std::vector<const ATM*> view;
    for (const auto& a : atms) view.push_back(a.get());
    Snapshot::setATMs(view);
    ConsoleUI::setSnapshotCallback([] { Snapshot::print(); });

    // 3. Main loop: pick an ATM, run one session on it, repeat.
    while (true) {
        std::cout << "\n===== ATM SYSTEM =====\n";
        for (size_t i = 0; i < system.atms.size(); ++i)
            std::cout << "  " << (i + 1) << ". ATM " << describe(system.atms[i]) << "\n";
        std::cout << "  0. Exit   (type / at any prompt for a snapshot)\n";

        try {
            int choice = ConsoleUI::readMenuChoice("Select an ATM: ", 0,
                                                   static_cast<int>(atms.size()));
            if (choice == 0) break;
            runOneSession(*atms[choice - 1]);
        } catch (const std::exception& e) {
            if (!std::cin) break; // input ended (e.g. piped test file)
            // Last line of defense so a bug never crashes the demo.
            std::cout << "Session ended: " << e.what() << "\n";
        }
    }

    std::cout << "Goodbye.\n";
    return 0;
}
