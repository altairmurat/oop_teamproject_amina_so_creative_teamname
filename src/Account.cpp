// Owner: Seunggyun (Person A)
#include "Account.h"
#include <cctype>
#include <stdexcept>
#include <utility>

Account::Account(std::string bankName, std::string ownerName, std::string accountNumber,
                 long long balance, std::string cardNumber, std::string password)
    : bankName(std::move(bankName)), ownerName(std::move(ownerName)),
      accountNumber(std::move(accountNumber)), balance(balance),
      cardNumber(std::move(cardNumber)), password(std::move(password)) {
    if (balance < 0) throw std::invalid_argument("Account balance cannot be negative");
}

std::string Account::normalize(const std::string& number) {
    std::string out;
    for (char c : number)
        if (c != '-' && !std::isspace(static_cast<unsigned char>(c))) out += c;
    return out;
}

bool Account::hasNumber(const std::string& number) const {
    return normalize(number) == normalize(accountNumber);
}

bool Account::hasCard(const std::string& number) const {
    return normalize(number) == normalize(cardNumber);
}

void Account::deposit(long long amount) {
    if (amount <= 0) throw std::invalid_argument("Deposit amount must be positive");
    balance += amount;
}

void Account::withdraw(long long amount) {
    if (amount <= 0) throw std::invalid_argument("Withdrawal amount must be positive");
    if (amount > balance) throw std::runtime_error("Insufficient funds in account " + accountNumber);
    balance -= amount;
}
