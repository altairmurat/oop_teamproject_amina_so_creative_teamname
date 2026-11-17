#include "CashInventory.h"
#include <algorithm>
#include <stdexcept>

CashInventory::CashInventory() {
    initEmpty();
}

CashInventory::CashInventory(const BillCount& initial) {
    initEmpty();
    addBills(initial);
}

CashInventory::CashInventory(int c50000, int c10000, int c5000, int c1000) {
    initEmpty();
    addBill(Constants::DENOM_50000, c50000);
    addBill(Constants::DENOM_10000, c10000);
    addBill(Constants::DENOM_5000,  c5000);
    addBill(Constants::DENOM_1000,  c1000);
}

void CashInventory::initEmpty() {
    for (int denom : Constants::DENOMINATIONS) {
        bills[denom] = 0;
    }
}

void CashInventory::addBills(const BillCount& added) {
    for (int denom : Constants::DENOMINATIONS) {
        addBill(denom, added.get(denom));
    }
}

void CashInventory::addBill(int denom, int count) {
    if (bills.find(denom) == bills.end()) {
        throw std::invalid_argument("unsupported bill denomination");
    }
    if (count < 0) {
        throw std::invalid_argument("bill count cannot be negative");
    }
    bills[denom] += count;
}

bool CashInventory::removeBills(const BillCount& removed) {
    for (int denom : Constants::DENOMINATIONS) {
        if (removed.get(denom) < 0 || bills.at(denom) < removed.get(denom)) {
            return false;
        }
    }
    for (int denom : Constants::DENOMINATIONS) {
        bills[denom] -= removed.get(denom);
    }
    return true;
}

int CashInventory::getCount(int denom) const {
    auto it = bills.find(denom);
    return (it != bills.end()) ? it->second : 0;
}

BillCount CashInventory::getBillCount() const {
    BillCount bc;
    bc.count50000 = getCount(Constants::DENOM_50000);
    bc.count10000 = getCount(Constants::DENOM_10000);
    bc.count5000  = getCount(Constants::DENOM_5000);
    bc.count1000  = getCount(Constants::DENOM_1000);
    return bc;
}

long long CashInventory::getTotalCash() const {
    long long sum = 0;
    for (const auto& [denom, count] : bills) {
        sum += static_cast<long long>(denom) * count;
    }
    return sum;
}

bool CashInventory::computeGreedyDispense(long long amount, BillCount& outPlan) const {
    if (amount <= 0 || amount % Constants::DENOM_1000 != 0) {
        return false;
    }

    BillCount plan;
    long long left = amount;

    for (const auto& [denom, avail] : bills) {
        if (left <= 0) break;
        long long need = left / denom;
        int take = static_cast<int>(std::min<long long>(need, avail));
        if (take > 0) {
            plan.set(denom, take);
            left -= static_cast<long long>(take) * denom;
        }
    }

    if (left != 0) {
        return false;
    }

    outPlan = plan;
    return true;
}

bool CashInventory::canDispense(long long amount) const {
    BillCount dummy;
    return computeGreedyDispense(amount, dummy);
}

bool CashInventory::tryDispense(long long amount, BillCount& dispensedOut) {
    BillCount plan;
    if (!computeGreedyDispense(amount, plan)) {
        return false;
    }
    removeBills(plan);
    dispensedOut = plan;
    return true;
}

BillCount CashInventory::dispense(long long amount) {
    BillCount out;
    if (!tryDispense(amount, out)) {
        // swap to InsufficientCashException once Exceptions.h is merged
        throw std::runtime_error("Insufficient cash in ATM.");
    }
    return out;
}
