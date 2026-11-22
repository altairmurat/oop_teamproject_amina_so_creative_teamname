#pragma once
#include "TransactionHistory.h"

class ATM;

class AdminMenu {
public:
    static void run(const ATM& atm, const TransactionHistory& history);
};
