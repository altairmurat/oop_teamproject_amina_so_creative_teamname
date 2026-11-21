// Amina's local test harness -- NOT part of the final program.
// usage: person_c_test <deposit|withdraw> <atm 1|2> <account 1|2> [priorWithdrawals] [topup]
// Loads data/initial_conditions.txt every run, so every test starts from the same state.
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

int main(int argc, char** argv) {
    if (argc < 4) { std::cerr << "usage: person_c_test <deposit|withdraw> <atm 1|2> <account 1|2> [prior] [topup]\n"; return 2; }
    const std::string mode = argv[1];
    const int atmIdx = std::stoi(argv[2]) - 1;
    const int accIdx = std::stoi(argv[3]) - 1;
    const int prior = argc > 4 ? std::stoi(argv[4]) : 0;
    const long long topup = argc > 5 ? std::stoll(argv[5]) : 0;

    SystemData sys = SystemLoader::load("data/initial_conditions.txt");
    std::vector<std::unique_ptr<ATM>> atms;
    for (const ATMConfig& c : sys.atms) {
        LanguageMode lm = c.bilingual ? LanguageMode::Bilingual : LanguageMode::Unilingual;
        CashInventory cash(c.bills[0], c.bills[1], c.bills[2], c.bills[3]);
        if (c.multiBank) atms.push_back(std::make_unique<MultiBankATM>(c.serialNumber, c.primaryBank, lm, cash));
        else             atms.push_back(std::make_unique<SingleBankATM>(c.serialNumber, c.primaryBank, lm, cash));
    }
    std::vector<const ATM*> view;
    for (auto& a : atms) view.push_back(a.get());
    Snapshot::setATMs(view);

    const char* accNums[] = {"100-100-111111", "200-200-222222"};
    Account* acc = Bank::findAccount(accNums[accIdx]);
    if (topup > 0) Bank::findBank(acc->getBankName())->deposit(*acc, topup);   // test-only top-up

    std::cout << "--- before ---";
    Snapshot::print();
    try {
        std::unique_ptr<Transaction> t;
        if (mode == "deposit") t = std::make_unique<DepositTransaction>(*atms[atmIdx], *acc);
        else                   t = std::make_unique<WithdrawalTransaction>(*atms[atmIdx], *acc, prior);
        t->execute();
        std::cout << "[history] #" << t->transactionID() << " " << t->type() << ": " << t->describe()
                  << " (fee " << t->transactionFee() << ")\n";
    } catch (const ATMException& e) {
        std::cout << e.what() << "\n[session would end here]\n";
    }
    std::cout << "--- after ---";
    Snapshot::print();
    return 0;
}
