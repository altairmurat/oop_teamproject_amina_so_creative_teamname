// Owner: Rabia (Person A)
#include "SystemLoader.h"
#include <cctype>
#include <iostream>
#include <sstream>
#include "Constants.h"

using namespace std;

const string SystemLoader::DEFAULT_ADMIN_CARD = "0000";

// this is to get the next "real" line (not blank, not a '#' comment), already trimmed.
// lineNumber goes up so error messages can say exactly which line is wrong
string SystemLoader::nextLine(ifstream& file, int& lineNumber, const string& expected) {
    string line;
    while (getline(file, line)) {
        lineNumber++;
        line = trim(line);
        if (!line.empty() && line[0] != '#') {
            return line;
        }
    }
    throw runtime_error("Unexpected end of file: expected " + expected);
}

runtime_error SystemLoader::lineError(int lineNumber, const string& message, const string& lineText) {
    return runtime_error("Line " + to_string(lineNumber) + ": " + message +
                         " (got \"" + lineText + "\")");
}

// this is to cut spaces, tabs and '\r' (Windows line endings) from both ends
string SystemLoader::trim(const string& text) {
    size_t begin = 0;
    size_t end = text.length();
    while (begin < end && isspace(static_cast<unsigned char>(text[begin]))) {
        begin++;
    }
    while (end > begin && isspace(static_cast<unsigned char>(text[end - 1]))) {
        end--;
    }
    return text.substr(begin, end - begin);
}

// this is to accept different spellings: "Single-Bank", "single bank", "SINGLE" -> "singlebank"/"single"
string SystemLoader::simplify(const string& text) {
    string result;
    for (size_t i = 0; i < text.length(); i++) {
        char c = text[i];
        if (c != '-' && c != '_' && !isspace(static_cast<unsigned char>(c))) {
            result += static_cast<char>(tolower(static_cast<unsigned char>(c)));
        }
    }
    return result;
}

bool SystemLoader::isAllDigits(const string& text) {
    if (text.empty()) {
        return false;
    }
    for (size_t i = 0; i < text.length(); i++) {
        if (!isdigit(static_cast<unsigned char>(text[i]))) {
            return false;
        }
    }
    return true;
}

void SystemLoader::load(const string& path, vector<Bank*>& banks, vector<ATMConfig>& atmConfigs) {
    ifstream file(path);
    if (!file) {
        throw runtime_error("Cannot open initial condition file: " + path);
    }
    Bank::clearAdminCards(); // this is so admin cards from an older load don't stay around

    int lineNumber = 0;
    string line;
    string extra; // used to detect extra stuff at the end of a line

    // ---- header: B A N ----
    line = nextLine(file, lineNumber, "header \"B A N\"");
    istringstream header(line);
    long long bankCount, accountCount, atmCount;
    if (!(header >> bankCount >> accountCount >> atmCount) || (header >> extra)) {
        throw lineError(lineNumber, "header must be three integers \"B A N\"", line);
    }
    if (bankCount < 1 || accountCount < 0 || atmCount < 1) {
        throw lineError(lineNumber, "need at least 1 bank and 1 ATM, and a non-negative account count", line);
    }

    // ---- banks: 1 line each ----
    for (long long i = 0; i < bankCount; i++) {
        line = nextLine(file, lineNumber, "bank name");
        if (Bank::findBank(line) != nullptr) {
            throw lineError(lineNumber, "duplicate bank name", line);
        }
        banks.push_back(new Bank(line)); // push right away so main can delete it if we fail later
    }

    // ---- accounts: 6 lines each ----
    for (long long i = 0; i < accountCount; i++) {
        // 1) bank of the account, has to be one of the banks above
        line = nextLine(file, lineNumber, "account bank name");
        Bank* bank = Bank::findBank(line);
        if (bank == nullptr) {
            throw lineError(lineNumber, "unknown bank", line);
        }

        // 2) user name - same user can appear many times (many accounts per user)
        string ownerName = nextLine(file, lineNumber, "user name");

        // 3) account number: 12 digits, dashes are allowed ("100-100-111111")
        string accountNumber = nextLine(file, lineNumber, "account number");
        string digits = Account::removeDashes(accountNumber);
        if (!isAllDigits(digits) || digits.length() != Constants::ACCOUNT_NUMBER_DIGITS) {
            throw lineError(lineNumber, "account number must have exactly 12 digits", accountNumber);
        }
        if (Bank::findAccount(accountNumber) != nullptr) {
            throw lineError(lineNumber, "duplicate account number", accountNumber);
        }

        // 4) funds, we also accept "4,000"
        line = nextLine(file, lineNumber, "available funds");
        string funds;
        for (size_t c = 0; c < line.length(); c++) {
            if (line[c] != ',') {
                funds += line[c];
            }
        }
        if (!isAllDigits(funds) || funds.length() > 15) {
            throw lineError(lineNumber, "funds must be a non-negative integer", line);
        }
        long long balance = stoll(funds);

        // 5) card number - one card opens exactly one account
        string cardNumber = nextLine(file, lineNumber, "card number");
        if (Bank::findAccountByCard(cardNumber) != nullptr) {
            throw lineError(lineNumber, "duplicate card number", cardNumber);
        }

        // 6) password
        string password = nextLine(file, lineNumber, "password");

        bank->openAccount(ownerName, accountNumber, balance, cardNumber, password);
    }

    // ---- ATMs: 5 lines each ----
    vector<string> serialNumbers; // this is to make sure no two ATMs get the same serial
    for (long long i = 0; i < atmCount; i++) {
        ATMConfig config;

        // 1) primary bank
        line = nextLine(file, lineNumber, "ATM primary bank");
        if (Bank::findBank(line) == nullptr) {
            throw lineError(lineNumber, "unknown bank", line);
        }
        config.primaryBank = line;

        // 2) serial number: exactly 6 digits and unique
        line = nextLine(file, lineNumber, "ATM serial number");
        if (!isAllDigits(line) || line.length() != Constants::SERIAL_NUMBER_LENGTH) {
            throw lineError(lineNumber, "serial number must be exactly 6 digits", line);
        }
        for (size_t s = 0; s < serialNumbers.size(); s++) {
            if (serialNumbers[s] == line) {
                throw lineError(lineNumber, "duplicate ATM serial number", line);
            }
        }
        serialNumbers.push_back(line);
        config.serialNumber = line;

        // 3) type
        line = nextLine(file, lineNumber, "ATM type");
        string type = simplify(line);
        if (type == "single" || type == "singlebank") {
            config.isMultiBank = false;
        } else if (type == "multi" || type == "multibank") {
            config.isMultiBank = true;
        } else {
            throw lineError(lineNumber, "type must be Single or Multi", line);
        }

        // 4) language
        line = nextLine(file, lineNumber, "ATM language");
        string language = simplify(line);
        if (language == "unilingual" || language == "uni") {
            config.isBilingual = false;
        } else if (language == "bilingual" || language == "bi") {
            config.isBilingual = true;
        } else {
            throw lineError(lineNumber, "language must be Unilingual or Bilingual", line);
        }

        // 5) cash: 4 counts in the order 50k 10k 5k 1k. "2, 3, 1, 3" and "2 / 3 / 1 / 3" also work
        line = nextLine(file, lineNumber, "ATM cash counts");
        string cashText = line;
        for (size_t c = 0; c < cashText.length(); c++) {
            if (cashText[c] == ',' || cashText[c] == '/') {
                cashText[c] = ' ';
            }
        }
        istringstream cash(cashText);
        for (int d = 0; d < 4; d++) {
            string count;
            if (!(cash >> count) || !isAllDigits(count) || count.length() > 9) {
                throw lineError(lineNumber, "cash must be 4 non-negative counts: 50000 10000 5000 1000", line);
            }
            config.bills[d] = stoi(count);
        }
        if (cash >> extra) {
            throw lineError(lineNumber, "cash must be exactly 4 counts", line);
        }

        atmConfigs.push_back(config);
    }

    // ---- optional admin cards: "Admin <card number>" (REQ1.9) ----
    bool hasAdminLine = false;
    while (getline(file, line)) {
        lineNumber++;
        line = trim(line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        istringstream adminLine(line);
        string word, cardNumber;
        if ((adminLine >> word >> cardNumber) && simplify(word) == "admin" && !(adminLine >> extra)) {
            // an admin card can't also be somebody's normal card
            if (Bank::findAccountByCard(cardNumber) != nullptr) {
                throw lineError(lineNumber, "admin card number is already a user card", line);
            }
            Bank::addAdminCard(cardNumber);
            hasAdminLine = true;
        } else {
            // not fatal, but we tell the user so a typo in the file doesn't go unnoticed
            cerr << "[warning] line " << lineNumber << " ignored: \"" << line << "\"\n";
        }
    }
    // no Admin line in the file -> use the default one (if no user already has that card)
    if (!hasAdminLine && Bank::findAccountByCard(DEFAULT_ADMIN_CARD) == nullptr) {
        Bank::addAdminCard(DEFAULT_ADMIN_CARD);
    }
}
