// Owner: Seunggyun (Person A)
#pragma once
#include <string>
#include <vector>

// One bank account (REQ1.6).
// Encapsulation: the balance can only change through deposit() and withdraw(),
// and the password is never returned, only checked.
class Account {
private:
    std::string m_bankName;
    std::string m_ownerName;
    std::string m_accountNumber; // stored as written in the file, e.g. "100-100-111111"
    long long m_balance;
    std::string m_cardNumber;
    std::string m_password;
    std::vector<std::string> m_history; // one line per successful transaction

public:
    Account(const std::string& bankName, const std::string& ownerName,
            const std::string& accountNumber, long long balance,
            const std::string& cardNumber, const std::string& password);

    std::string getBankName() const { return m_bankName; }
    std::string getOwnerName() const { return m_ownerName; }
    std::string getAccountNumber() const { return m_accountNumber; }
    std::string getCardNumber() const { return m_cardNumber; }
    long long getBalance() const { return m_balance; }
    std::vector<std::string> getHistory() const { return m_history; }

    bool checkPassword(const std::string& password) const;
    bool hasAccountNumber(const std::string& accountNumber) const; // ignores '-' and spaces
    bool hasCardNumber(const std::string& cardNumber) const;       // ignores '-' and spaces
    bool canWithdraw(long long amount) const;

    // Throw InvalidInputException / InsufficientFundsException (Exceptions.h)
    // and leave the balance unchanged if the request is not allowed.
    void deposit(long long amount);
    void withdraw(long long amount);

    void addHistory(const std::string& line);

    // "100-100-111111" -> "100100111111". Static: it does not belong to one account.
    static std::string removeDashes(const std::string& number);
};
