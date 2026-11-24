// Owner: Seunggyun (Person A)
#include "Account.h"
#include "Exceptions.h"

using namespace std;

Account::Account(const string& bankName, const string& ownerName,
                 const string& accountNumber, long long balance,
                 const string& cardNumber, const string& password)
    : m_bankName{ bankName }, m_ownerName{ ownerName },
      m_accountNumber{ accountNumber }, m_balance{ balance },
      m_cardNumber{ cardNumber }, m_password{ password } {
    // a bank account can't start with negative money
    if (balance < 0) {
        throw InvalidInputException("Account balance cannot be negative: " + accountNumber);
    }
}

bool Account::checkPassword(const string& password) const {
    return password == m_password;
}

bool Account::hasAccountNumber(const string& accountNumber) const {
    return removeDashes(accountNumber) == removeDashes(m_accountNumber);
}

bool Account::hasCardNumber(const string& cardNumber) const {
    return removeDashes(cardNumber) == removeDashes(m_cardNumber);
}

// this is to check BEFORE changing anything (transactions use it to validate first)
bool Account::canWithdraw(long long amount) const {
    return amount > 0 && amount <= m_balance;
}

void Account::deposit(long long amount) {
    if (amount <= 0) {
        throw InvalidInputException("Deposit amount must be positive.");
    }
    m_balance += amount;
}

void Account::withdraw(long long amount) {
    if (amount <= 0) {
        throw InvalidInputException("Withdrawal amount must be positive.");
    }
    // not enough money -> throw and leave the balance as it was
    if (amount > m_balance) {
        throw InsufficientFundsException("Insufficient funds in account " + m_accountNumber + ".");
    }
    m_balance -= amount;
}

void Account::addHistory(const string& line) {
    m_history.push_back(line);
}

string Account::removeDashes(const string& number) {
    string result;
    for (size_t i = 0; i < number.length(); i++) {
        // keep only the real characters, skip '-' and spaces
        if (number[i] != '-' && number[i] != ' ') {
            result += number[i];
        }
    }
    return result;
}
