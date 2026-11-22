// Owner: Seunggyun (Person A)
#include "Card.h"

Card::Card(const std::string& cardNumber, const std::string& bankName)
    : m_cardNumber{ cardNumber }, m_bankName{ bankName } {
}

// An admin card is not issued by any bank, so the base part gets an empty bank name.
AdminCard::AdminCard(const std::string& cardNumber)
    : Card(cardNumber, "") {
}
