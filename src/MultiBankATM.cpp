// Owner: Altair (Person B)
#include "MultiBankATM.h"
#include "Card.h"

bool MultiBankATM::isCardValid(const Card& card) const {
    if (card.isAdmin()) {
        return true;
    }
    // multi-bank ATM accepts cards from any registered bank
    return !card.getBankName().empty();
}
