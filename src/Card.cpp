// Owner: Seunggyun (Person A)
#include "Card.h"
#include <utility>

Card::Card(std::string cardNumber, std::string bankName)
    : cardNumber(std::move(cardNumber)), bankName(std::move(bankName)) {}

AdminCard::AdminCard(std::string cardNumber)
    : Card(std::move(cardNumber), "") {}
