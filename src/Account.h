// Owner: Seunggyun (Person A)
#pragma once
#include <string>
#include <vector>

// One bank account (REQ1.6). The password is never exposed, only checked.
// Balance changes go through deposit()/withdraw(), which enforce the rules.
class Account {
public:
    Account(std::string bankName, std::string ownerName, std::string accountNumber,
            long long balance, std::string cardNumber, std::string password);

    const std::string& getBankName() const { return bankName; }
    const std::string& getOwnerName() const { return ownerName; }
    const std::string& getAccountNumber() const { return accountNumber; } // as written in the file
    const std::string& getCardNumber() const { return cardNumber; }
    long long getBalance() const { return balance; }

    bool checkPassword(const std::string& pw) const { return pw == password; }
    bool hasNumber(const std::string& number) const;   // dash/space-insensitive
    bool hasCard(const std::string& number) const;     // dash/space-insensitive
    bool canWithdraw(long long amount) const { return amount > 0 && amount <= balance; }

    // Callers (transactions) validate first. These throw std::invalid_argument /
    // std::runtime_error only as a safety net for programming errors.
    void deposit(long long amount);   // amount > 0
    void withdraw(long long amount);  // 0 < amount <= balance

    // Per-account history of successful transactions (one line each).
    void addHistory(const std::string& line) { history.push_back(line); }
    const std::vector<std::string>& getHistory() const { return history; }

    // "100-100-111111" -> "100100111111"
    static std::string normalize(const std::string& number);

private:
    std::string bankName;
    std::string ownerName;
    std::string accountNumber;
    long long balance;
    std::string cardNumber;
    std::string password;
    std::vector<std::string> history;
};
