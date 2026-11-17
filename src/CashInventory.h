#pragma once

#include <functional>
#include <map>
#include "Constants.h"

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

    bool canDispense(long long amount) const;
    bool tryDispense(long long amount, BillCount& dispensedOut);
    BillCount dispense(long long amount);

private:
    std::map<int, int, std::greater<int>> bills;

    void initEmpty();
    bool computeGreedyDispense(long long amount, BillCount& outPlan) const;
};
