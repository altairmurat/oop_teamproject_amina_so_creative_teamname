// Owner: Seunggyun (Person A)
#pragma once
#include <array>
#include <memory>
#include <string>
#include <vector>
#include "Bank.h"

// What the file says about one ATM. main() turns these into real ATM objects
// (Person B's classes), so this loader does not depend on the ATM code.
struct ATMConfig {
    std::string primaryBank;
    std::string serialNumber;   // exactly 6 digits
    bool multiBank = false;     // false: Single-Bank, true: Multi-Bank
    bool bilingual = false;     // false: Unilingual, true: Bilingual
    std::array<int, 4> bills{}; // counts of 50000 / 10000 / 5000 / 1000
};

struct SystemData {
    std::vector<std::unique_ptr<Bank>> banks; // accounts live inside the banks
    std::vector<ATMConfig> atms;
};

// Parses the initial condition file (REQ1.11). Format:
//   line 1      : B A N            (number of banks, accounts, ATMs)
//   B lines     : bank name
//   6 x A lines : bank, user name, account number, funds, card number, password
//   5 x N lines : primary bank, serial (6 digits), Single|Multi, Unilingual|Bilingual,
//                 "c50k c10k c5k c1k"
//   optional    : "Admin <card number>" lines
// Blank lines and lines starting with '#' are ignored. On any error a
// std::runtime_error with the line number is thrown.
class SystemLoader {
public:
    static SystemData load(const std::string& path);

    // Used when the file declares no admin card (team decision, see README).
    static const std::string DEFAULT_ADMIN_CARD;
};
