// Owner: Seunggyun (Person A)
#include "Bank.h"
#include <algorithm>
#include <stdexcept>
#include <utility>

// Function-local statics: no static-initialization-order problems.
std::vector<Bank*>& Bank::registry() {
    static std::vector<Bank*> banks;
    return banks;
}

std::set<std::string>& Bank::adminCards() {
    static std::set<std::string> cards;
    return cards;
}

Bank::Bank(std::string bankName) : name(std::move(bankName)) {
    if (name.empty()) throw std::invalid_argument("Bank name cannot be empty");
    if (findBank(name)) throw std::invalid_argument("Duplicate bank name: " + name);
    registry().push_back(this);
}

Bank::~Bank() {
    auto& r = registry();
    r.erase(std::remove(r.begin(), r.end(), this), r.end());
}

Account& Bank::openAccount(const std::string& owner, const std::string& accountNumber,
                           long long balance, const std::string& cardNumber,
                           const std::string& password) {
    if (findAccount(accountNumber))
        throw std::invalid_argument("Duplicate account number: " + accountNumber);
    accounts.push_back(std::make_unique<Account>(name, owner, accountNumber, balance,
                                                 cardNumber, password));
    return *accounts.back();
}

std::vector<Account*> Bank::getAccounts() const {
    std::vector<Account*> out;
    for (const auto& a : accounts) out.push_back(a.get());
    return out;
}

bool Bank::owns(const Account& acc) const {
    for (const auto& a : accounts)
        if (a.get() == &acc) return true;
    return false;
}

Account* Bank::getAccountByCard(const std::string& cardNumber) const {
    for (const auto& a : accounts)
        if (a->hasCard(cardNumber)) return a.get();
    return nullptr;
}

bool Bank::verifyCard(const std::string& cardNumber) const {
    return getAccountByCard(cardNumber) != nullptr;
}

bool Bank::verifyPassword(const std::string& cardNumber, const std::string& pw) const {
    Account* a = getAccountByCard(cardNumber);
    return a != nullptr && a->checkPassword(pw);
}

void Bank::deposit(Account& acc, long long amount) {
    if (!owns(acc)) throw std::invalid_argument("Account does not belong to " + name);
    acc.deposit(amount);
}

void Bank::withdraw(Account& acc, long long amount) {
    if (!owns(acc)) throw std::invalid_argument("Account does not belong to " + name);
    acc.withdraw(amount);
}

Account* Bank::findAccount(const std::string& accountNumber) {
    for (Bank* b : registry())
        for (const auto& a : b->accounts)
            if (a->hasNumber(accountNumber)) return a.get();
    return nullptr;
}

Account* Bank::findAccountByCard(const std::string& cardNumber) {
    for (Bank* b : registry())
        if (Account* a = b->getAccountByCard(cardNumber)) return a;
    return nullptr;
}

Bank* Bank::findBank(const std::string& bankName) {
    for (Bank* b : registry())
        if (b->name == bankName) return b;
    return nullptr;
}

const std::vector<Bank*>& Bank::allBanks() { return registry(); }

void Bank::transfer(Account& src, Account& dst, long long amount, long long fee) {
    if (&src == &dst) throw std::invalid_argument("Source and destination are the same account");
    if (amount <= 0 || fee < 0) throw std::invalid_argument("Invalid transfer amount or fee");
    if (src.getBalance() < amount + fee)
        throw std::runtime_error("Insufficient funds in account " + src.getAccountNumber());
    src.withdraw(amount + fee);  // cannot fail after the check above
    dst.deposit(amount);
}

void Bank::registerAdminCard(const std::string& cardNumber) {
    adminCards().insert(Account::normalize(cardNumber));
}

bool Bank::isAdminCard(const std::string& cardNumber) {
    return adminCards().count(Account::normalize(cardNumber)) > 0;
}

std::unique_ptr<Card> Bank::lookupCard(const std::string& cardNumber) {
    if (isAdminCard(cardNumber)) return std::make_unique<AdminCard>(cardNumber);
    if (Account* a = findAccountByCard(cardNumber))
        return std::make_unique<Card>(cardNumber, a->getBankName());
    return nullptr;
}
