// Owner: Seunggyun (Person A)
#pragma once
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "Bank.h"

// What the file says about one ATM. main() turns it into a SingleBankATM or
// MultiBankATM object, so the loader does not need the ATM classes.
struct ATMConfig {
    std::string primaryBank;
    std::string serialNumber;
    bool isMultiBank = false;    // false: Single-Bank, true: Multi-Bank
    bool isBilingual = false;    // false: Unilingual, true: Bilingual
    int bills[4] = { 0, 0, 0, 0 }; // counts of 50000, 10000, 5000, 1000 KRW bills
};

// Reads the initial condition file (REQ1.11). File format:
//   line 1      : B A N            (number of banks, accounts, ATMs)
//   B lines     : bank name
//   6 x A lines : bank, user name, account number, funds, card number, password
//   5 x N lines : primary bank, serial number, Single|Multi, Unilingual|Bilingual,
//                 "count50000 count10000 count5000 count1000"
//   optional    : "Admin <card number>" lines
// Blank lines and lines starting with '#' are skipped.
class SystemLoader {
public:
    // Fills banks (created with new; the caller must delete them, also after an
    // exception) and atmConfigs. Throws std::runtime_error with the line number
    // if the file is missing or not in the format above.
    static void load(const std::string& path, std::vector<Bank*>& banks,
                     std::vector<ATMConfig>& atmConfigs);

    // Admin card used when the file has no "Admin" line.
    static const std::string DEFAULT_ADMIN_CARD;

private:
    static std::string nextLine(std::ifstream& file, int& lineNumber, const std::string& expected);
    static std::runtime_error lineError(int lineNumber, const std::string& message,
                                        const std::string& lineText);
    static std::string trim(const std::string& text);
    static std::string simplify(const std::string& text);
    static bool isAllDigits(const std::string& text);
};
