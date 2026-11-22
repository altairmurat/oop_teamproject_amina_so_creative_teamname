// Owner: Seunggyun (Person A)
#pragma once
#include <string>

// Base class of every card that can be inserted into an ATM.
// A card only knows its number and the bank that issued it.
// All account data stays inside the Bank (REQ1.5).
class Card {
private:
    std::string m_cardNumber;
    std::string m_bankName; // "" for an admin card

public:
    Card(const std::string& cardNumber, const std::string& bankName);
    virtual ~Card() {} // virtual: a derived card may be used through a Card pointer

    std::string getCardNumber() const { return m_cardNumber; }
    std::string getBankName() const { return m_bankName; }

    // Overridden by AdminCard. Called through a Card reference (dynamic binding).
    virtual bool isAdmin() const { return false; }
};

// REQ1.9: an admin card opens the admin menu instead of a normal session.
// It is a Card ("is-a" relationship), so it inherits publicly from Card.
class AdminCard : public Card {
public:
    AdminCard(const std::string& cardNumber);

    bool isAdmin() const override { return true; }
};
