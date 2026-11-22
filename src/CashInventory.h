// Owner: Altair (Person B)
#pragma once
#include <functional>
#include <map>
#include "Constants.h"

// this is to manage the physical bills inside an ATM
class CashInventory {
public:
    CashInventory();
    explicit CashInventory(const BillCount& initial);
    CashInventory(int c50000, int c10000, int c5000, int c1000);

    void addBills(const BillCount& added);
    void addBill(int denom, int count);
    bool removeBills(const BillCount& removed);

    int getCount(int denom) const;
    BillCount getBillCount() const;
    long long getTotalCash() const;

    // checks if we can make the exact amount with the bills currently in the ATM
    bool canDispense(long long amount) const;

    // greedy dispense: removes bills and returns true only if exact change is possible
    bool tryDispense(long long amount, BillCount& dispensedOut);
    BillCount dispense(long long amount);

private:
    // std::greater keeps keys sorted 50000 -> 10000 -> 5000 -> 1000
    std::map<int, int, std::greater<int>> bills;

    void initEmpty();
    bool computeGreedyDispense(long long amount, BillCount& outPlan) const;
};
