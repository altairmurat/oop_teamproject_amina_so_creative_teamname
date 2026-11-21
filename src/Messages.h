// Alan
#pragma once
#include <map>
#include <string>
#include "Constants.h"

class Messages {
public:
    static std::string get(const std::string& key, Language /*lang*/) {
        static const std::map<std::string, std::string> en = {
            {"prompt.bills_of", "Number of bills for KRW "},
            {"err.too_many_bills", "Error: too many bills (maximum 50 per transaction)."},
            {"deposit.choose_type", "Deposit type - 1) Cash  2) Check: "},
            {"deposit.enter_check", "Enter the check amount (KRW 100,000 or more): "},
            {"deposit.fee_label", "Deposit fee required - insert it in cash (KRW): "},
            {"err.check_too_small", "Error: invalid check (minimum KRW 100,000)."},
            {"err.no_cash", "Error: no cash was inserted."},
            {"err.fee_mismatch", "Error: the inserted fee does not match the required fee."},
            {"deposit.success", "Deposit completed."},
            {"withdraw.enter_amount", "Enter the withdrawal amount (KRW): "},
            {"err.withdraw_count", "Error: maximum number of withdrawals per session (3) reached."},
            {"err.withdraw_max", "Error: maximum withdrawal per transaction is KRW 500,000."},
            {"err.amount_unit", "Error: amount must be a multiple of KRW 1,000."},
            {"err.no_funds", "Error: insufficient funds in the account (amount + fee)."},
            {"err.no_cash_atm", "Error: insufficient cash in the ATM."},
            {"err.too_many_dispense", "Error: dispensing this amount would exceed 50 bills."},
            {"withdraw.dispensing", "Dispensing:"},
            {"withdraw.fee_label", "Fee charged to the account (KRW): "},
            {"withdraw.success", "Withdrawal completed."}};
        auto it = en.find(key);
        return it == en.end() ? key : it->second;
    }
};
