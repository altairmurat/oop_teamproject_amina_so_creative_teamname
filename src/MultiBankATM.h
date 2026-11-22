// Owner: Altair (Person B)
#pragma once
#include "ATM.h"

class MultiBankATM : public ATM {
public:
    using ATM::ATM;

    bool isCardValid(const Card& card) const override;
    std::string getType() const override { return "Multi"; }
};
