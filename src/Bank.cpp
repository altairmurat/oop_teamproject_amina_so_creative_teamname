// Owner: Seunggyun (Person A)
#include "Bank.h"
#include "Exceptions.h"

using namespace std;

// static members have to be defined once outside the class
vector<Bank*> Bank::m_allBanks;
vector<string> Bank::m_adminCards;

Bank::Bank(const string& name) : m_name{ name } {
    if (name.empty()) {
        throw InvalidInputException("Bank name cannot be empty.");
    }
    if (findBank(name) != nullptr) {
        throw InvalidInputException("Duplicate bank name: " + name);
    }
    m_allBanks.push_back(this); // register this bank so the static searches can see it
}

Bank::~Bank() {
    // the bank owns its accounts, so it deletes them
    for (size_t i = 0; i < m_accounts.size(); i++) {
        delete m_accounts[i];
    }
    m_accounts.clear();

    // this is to take the bank out of the global list, otherwise we'd keep a dangling pointer
    for (size_t i = 0; i < m_allBanks.size(); i++) {
        if (m_allBanks[i] == this) {
            m_allBanks.erase(m_allBanks.begin() + i);
            break;
        }
    }
}

void Bank::openAccount(const string& ownerName, const string& accountNumber,
                       long long balance, const string& cardNumber,
                       const string& password) {
    // account numbers must be unique in the whole system, not only in this bank
    if (findAccount(accountNumber) != nullptr) {
        throw InvalidInputException("Duplicate account number: " + accountNumber);
    }
    // one card = one account, otherwise we wouldn't know which account the card opens
    if (findAccountByCard(cardNumber) != nullptr) {
        throw InvalidInputException("Duplicate card number: " + cardNumber);
    }
    Account* account = new Account(m_name, ownerName, accountNumber, balance, cardNumber, password);
    m_accounts.push_back(account);
}

bool Bank::hasAccount(const Account& account) const {
    for (size_t i = 0; i < m_accounts.size(); i++) {
        if (m_accounts[i] == &account) {
            return true;
        }
    }
    return false;
}

Account* Bank::getAccountByCard(const string& cardNumber) const {
    for (size_t i = 0; i < m_accounts.size(); i++) {
        if (m_accounts[i]->hasCardNumber(cardNumber)) {
            return m_accounts[i];
        }
    }
    return nullptr;
}

// this is to answer "did this bank issue this card?"
bool Bank::verifyCard(const string& cardNumber) const {
    return getAccountByCard(cardNumber) != nullptr;
}

// this is to answer "is the password right for this card?" - the ATM never sees the real password
bool Bank::verifyPassword(const string& cardNumber, const string& password) const {
    Account* account = getAccountByCard(cardNumber);
    if (account == nullptr) {
        return false;
    }
    return account->checkPassword(password);
}

void Bank::deposit(Account& account, long long amount) {
    // a bank can only change money in its own accounts
    if (!hasAccount(account)) {
        throw InvalidInputException("Account " + account.getAccountNumber() +
                                    " does not belong to " + m_name + ".");
    }
    account.deposit(amount);
}

void Bank::withdraw(Account& account, long long amount) {
    if (!hasAccount(account)) {
        throw InvalidInputException("Account " + account.getAccountNumber() +
                                    " does not belong to " + m_name + ".");
    }
    account.withdraw(amount);
}

// this is to find an account in ANY bank (used for transfer destinations)
Account* Bank::findAccount(const string& accountNumber) {
    for (size_t b = 0; b < m_allBanks.size(); b++) {
        vector<Account*>& accounts = m_allBanks[b]->m_accounts;
        for (size_t a = 0; a < accounts.size(); a++) {
            if (accounts[a]->hasAccountNumber(accountNumber)) {
                return accounts[a];
            }
        }
    }
    return nullptr;
}

// this is to find which account (and so which bank) a card belongs to
Account* Bank::findAccountByCard(const string& cardNumber) {
    for (size_t b = 0; b < m_allBanks.size(); b++) {
        Account* account = m_allBanks[b]->getAccountByCard(cardNumber);
        if (account != nullptr) {
            return account;
        }
    }
    return nullptr;
}

Bank* Bank::findBank(const string& name) {
    for (size_t b = 0; b < m_allBanks.size(); b++) {
        if (m_allBanks[b]->m_name == name) {
            return m_allBanks[b];
        }
    }
    return nullptr;
}

vector<Bank*> Bank::getAllBanks() {
    return m_allBanks;
}

void Bank::transfer(Account& source, Account& destination, long long amount, long long fee) {
    if (&source == &destination) {
        throw InvalidInputException("Source and destination are the same account.");
    }
    if (amount <= 0 || fee < 0) {
        throw InvalidInputException("Invalid transfer amount or fee.");
    }
    // check first, change later - so a failed transfer never leaves half the money moved
    if (source.getBalance() < amount + fee) {
        throw InsufficientFundsException("Insufficient funds in account " +
                                         source.getAccountNumber() + ".");
    }
    source.withdraw(amount + fee); // can't fail anymore because of the check above
    destination.deposit(amount);
}

void Bank::addAdminCard(const string& cardNumber) {
    if (!isAdminCard(cardNumber)) {
        m_adminCards.push_back(Account::removeDashes(cardNumber));
    }
}

bool Bank::isAdminCard(const string& cardNumber) {
    string number = Account::removeDashes(cardNumber);
    for (size_t i = 0; i < m_adminCards.size(); i++) {
        if (m_adminCards[i] == number) {
            return true;
        }
    }
    return false;
}

void Bank::clearAdminCards() {
    m_adminCards.clear();
}
