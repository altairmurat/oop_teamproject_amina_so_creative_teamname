// Owner: Rabia (Person A)
#include "Card.h"

Card::Card(const std::string& cardNumber, const std::string& bankName)
    : m_cardNumber{ cardNumber }, m_bankName{ bankName } {
}

// admin card is not issued by any bank, so we pass an empty bank name to the base part
AdminCard::AdminCard(const std::string& cardNumber)
    : Card(cardNumber, "") {
}
