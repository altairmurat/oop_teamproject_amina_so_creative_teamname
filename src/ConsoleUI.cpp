// Owner: Altair (Person B)
#include "ConsoleUI.h"
#include "ATM.h"
#include "Exceptions.h"
#include "Messages.h"
#include <cctype>
#include <limits>
#include <stdexcept>

std::function<void()> ConsoleUI::snapshotCallback = nullptr;
Language ConsoleUI::currentLanguage = Language::English;

void ConsoleUI::setSnapshotCallback(std::function<void()> callback) {
    snapshotCallback = std::move(callback);
}

void ConsoleUI::triggerSnapshot() {
    if (snapshotCallback) {
        snapshotCallback();
    }
}

void ConsoleUI::setLanguage(Language lang) {
    currentLanguage = lang;
}

Language ConsoleUI::getLanguage() {
    return currentLanguage;
}

std::string ConsoleUI::trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(start, end - start);
}

std::string ConsoleUI::readLine(const std::string& prompt) {
    while (true) {
        if (!prompt.empty()) {
            std::cout << prompt;
        }
        std::string line;
        if (!std::getline(std::cin, line)) {
            throw std::runtime_error("input stream closed");
        }
        std::string cleaned = trim(line);
        // this is to catch '/' at any prompt and print the ATM/account snapshot
        if (cleaned.size() == 1 && cleaned[0] == Constants::SNAPSHOT_COMMAND) {
            triggerSnapshot();
            continue;
        }
        return cleaned;
    }
}

int ConsoleUI::readMenuChoice(const std::string& prompt, int minVal, int maxVal) {
    return readInt(prompt, minVal, maxVal);
}

long long ConsoleUI::readPositiveAmount(const std::string& prompt) {
    return readLongLong(prompt, 1, std::numeric_limits<long long>::max());
}

int ConsoleUI::readInt(const std::string& prompt, int minVal, int maxVal) {
    while (true) {
        std::string line = readLine(prompt);
        if (line.empty()) {
            showMessage(Messages::get("invalid_input", currentLanguage));
            continue;
        }
        try {
            size_t pos = 0;
            long long val = std::stoll(line, &pos);
            if (pos != line.size() || val < minVal || val > maxVal) {
                showMessage(Messages::get("invalid_input", currentLanguage));
                continue;
            }
            return static_cast<int>(val);
        } catch (const std::exception&) {
            showMessage(Messages::get("invalid_input", currentLanguage));
        }
    }
}

long long ConsoleUI::readLongLong(const std::string& prompt, long long minVal, long long maxVal) {
    while (true) {
        std::string line = readLine(prompt);
        if (line.empty()) {
            showMessage(Messages::get("invalid_input", currentLanguage));
            continue;
        }
        try {
            size_t pos = 0;
            long long val = std::stoll(line, &pos);
            if (pos != line.size() || val < minVal || val > maxVal) {
                showMessage(Messages::get("invalid_input", currentLanguage));
                continue;
            }
            return val;
        } catch (const std::exception&) {
            showMessage(Messages::get("invalid_input", currentLanguage));
        }
    }
}

BillCount ConsoleUI::readBills(int maxTotalBills) {
    BillCount bills;
    for (int denom : Constants::DENOMINATIONS) {
        std::string prompt = Messages::get("prompt.bills_of", currentLanguage) + std::to_string(denom) + ": ";
        int cnt = readInt(prompt, 0, 1000000);
        bills.set(denom, cnt);
    }
    // enforce the 50-bill limit per transaction (REQ4.2)
    if (bills.totalBills() > maxTotalBills) {
        throw LimitExceededException(Messages::get("err.too_many_bills", currentLanguage));
    }
    return bills;
}

Language ConsoleUI::promptLanguage(const ATM& atm) {
    if (atm.getLanguageMode() == LanguageMode::Unilingual) {
        return Language::English;
    }
    // show both English and Korean prompts so the user can choose either language
    std::string prompt = Messages::get("select_language", Language::English) +
                         "\n" + Messages::get("select_language", Language::Korean);
    int choice = readMenuChoice(prompt, 1, 2);
    return (choice == 2) ? Language::Korean : Language::English;
}

void ConsoleUI::showMessage(const std::string& msg) {
    std::cout << msg << '\n';
}
