// Owner: Altair (Person B)
#include "SingleBankATM.h"
#include "Card.h"

bool SingleBankATM::isCardValid(const Card& card) const {
    // admin cards can be inserted into any ATM to check transaction history
    if (card.isAdmin()) {
        return true;
    }
    // single-bank ATM only accepts cards issued by its own primary bank
    return isPrimaryBank(card.getBankName());
}
