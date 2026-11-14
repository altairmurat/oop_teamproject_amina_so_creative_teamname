#pragma once
#include "TransactionHistory.h"
#include <string>

class ATM;

class AdminMenu {
public:
    static void run(const ATM& atm, const TransactionHistory& history);
    static std::string buildOutputFilename(const ATM& atm);
};
