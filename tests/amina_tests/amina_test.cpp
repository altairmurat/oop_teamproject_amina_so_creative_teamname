// Amina's (Person C) local test harness -- NOT part of the final program.
// usage: amina_test <deposit|withdraw> <atm 1|2> <account 1|2> [priorWithdrawals] [topup] [en|ko]
// Loads data/initial_conditions.txt every run, so every test starts from the same state.
// For tests through the full menu flow (card, password, session summary) use ./atm instead.
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include "ATM.h"
#include "Account.h"
#include "Bank.h"
#include "CashInventory.h"
#include "ConsoleUI.h"
#include "DepositTransaction.h"
#include "Exceptions.h"
#include "MultiBankATM.h"
#include "SingleBankATM.h"
#include "Snapshot.h"
#include "SystemLoader.h"
#include "WithdrawalTransaction.h"

// same job as createATM() in main.cpp
static ATM* makeATM(const ATMConfig& c) {
    LanguageMode mode = c.isBilingual ? LanguageMode::Bilingual : LanguageMode::Unilingual;
    CashInventory cash(c.bills[0], c.bills[1], c.bills[2], c.bills[3]);
    if (c.isMultiBank) return new MultiBankATM(c.serialNumber, c.primaryBank, mode, cash);
    return new SingleBankATM(c.serialNumber, c.primaryBank, mode, cash);
}

static void cleanUp(std::vector<ATM*>& atms, std::vector<Bank*>& banks) {
    for (ATM* a : atms) delete a;
    atms.clear();
    for (Bank* b : banks) delete b;   // a bank deletes its own accounts
    banks.clear();
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "usage: amina_test <deposit|withdraw> <atm 1|2> <account 1|2> [prior] [topup] [en|ko]\n";
        return 2;
    }
    const std::string mode = argv[1];
    const int atmIdx = std::stoi(argv[2]) - 1;
    const int accIdx = std::stoi(argv[3]) - 1;
    const int prior = argc > 4 ? std::stoi(argv[4]) : 0;
    const long long topup = argc > 5 ? std::stoll(argv[5]) : 0;
    const bool korean = argc > 6 && std::string(argv[6]) == "ko";

    std::vector<Bank*> banks;
    std::vector<ATMConfig> configs;
    std::vector<ATM*> atms;
    int exitCode = 0;
    try {
        SystemLoader::load("data/initial_conditions.txt", banks, configs);
        for (const ATMConfig& c : configs) atms.push_back(makeATM(c));
        if (atmIdx < 0 || atmIdx >= static_cast<int>(atms.size())) throw std::runtime_error("no such ATM");
        Snapshot::setATMs(atms);

        if (korean) atms[atmIdx]->setLanguage(Language::Korean);   // only works on a bilingual ATM

        const char* accNums[] = {"100-100-111111", "200-200-222222"};
        Account* acc = Bank::findAccount(accNums[accIdx]);
        if (!acc) throw std::runtime_error("no such account");
        if (topup > 0) Bank::findBank(acc->getBankName())->deposit(*acc, topup);   // test-only top-up

        std::cout << "--- before ---";
        Snapshot::print();
        try {
            std::unique_ptr<Transaction> t;
            if (mode == "deposit") t.reset(new DepositTransaction(*atms[atmIdx], *acc));
            else                   t.reset(new WithdrawalTransaction(*atms[atmIdx], *acc, prior));
            t->execute();
            std::cout << "[history] #" << t->transactionID() << " " << t->type() << ": " << t->describe()
                      << " (fee " << t->transactionFee() << ")\n";
        } catch (const ATMException& e) {
            std::cout << e.what() << "\n[session would end here]\n";
        }
        std::cout << "--- after ---";
        Snapshot::print();
    } catch (const std::exception& e) {
        std::cerr << "test setup failed: " << e.what() << "\n";
        exitCode = 1;
    }
    cleanUp(atms, banks);
    return exitCode;
}
