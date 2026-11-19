// Owner: Seunggyun (Person A)
#pragma once
#include <string>

// A card inserted into an ATM. Holds only the card number and the issuing bank;
// all account data stays inside the Bank (REQ1.5: ATMs store no user data).
class Card {
public:
    Card(std::string cardNumber, std::string bankName);
    virtual ~Card() = default;

    const std::string& getCardNumber() const { return cardNumber; }
    const std::string& getBankName() const { return bankName; } // "" for admin cards
    virtual bool isAdmin() const { return false; }

private:
    std::string cardNumber;
    std::string bankName;
};

// REQ1.9: an admin card opens the admin menu instead of a normal session.
class AdminCard : public Card {
public:
    explicit AdminCard(std::string cardNumber);
    bool isAdmin() const override { return true; }
};
