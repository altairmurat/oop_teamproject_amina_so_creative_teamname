// Owner: Seunggyun (Person A)
#pragma once
#include <string>

// this is the base class for every card you can put into an ATM.
// a card only knows its own number and which bank gave it out,
// all the real account data stays inside the Bank (REQ1.5)
class Card {
private:
    std::string m_cardNumber;
    std::string m_bankName; // empty string for an admin card

public:
    Card(const std::string& cardNumber, const std::string& bankName);
    virtual ~Card() {} // virtual so deleting through a Card* also cleans up a derived card

    std::string getCardNumber() const { return m_cardNumber; }
    std::string getBankName() const { return m_bankName; }

    // this is to let the Session ask "is this an admin card?" without knowing the real type.
    // AdminCard overrides it, so the call is decided at runtime (polymorphism)
    virtual bool isAdmin() const { return false; }
};

// REQ1.9: admin card opens the admin menu instead of the normal user menu.
// an admin card IS a card, so it inherits publicly from Card
class AdminCard : public Card {
public:
    AdminCard(const std::string& cardNumber);

    bool isAdmin() const override { return true; }
};
