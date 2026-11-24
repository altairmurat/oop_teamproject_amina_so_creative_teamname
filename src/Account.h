// Owner: Seunggyun (Person A)
#pragma once
#include <string>
#include <vector>

// this is one bank account (REQ1.6): bank, owner, 12-digit number, balance, card, password, history.
// encapsulation: the balance can only change through deposit() and withdraw(),
// and the password is never given out, we can only ask "is this the right one?"
class Account {
private:
    std::string m_bankName;
    std::string m_ownerName;
    std::string m_accountNumber; // kept exactly like in the file, e.g. "100-100-111111"
    long long m_balance;
    std::string m_cardNumber;
    std::string m_password;
    std::vector<std::string> m_history; // one line for every successful transaction

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

    // these throw InvalidInputException / InsufficientFundsException (from Exceptions.h)
    // and do NOT touch the balance if the request is wrong
    void deposit(long long amount);
    void withdraw(long long amount);

    void addHistory(const std::string& line);

    // this is to compare numbers no matter how they are typed: "100-100-111111" -> "100100111111".
    // static because it doesn't need any particular account
    static std::string removeDashes(const std::string& number);
};
