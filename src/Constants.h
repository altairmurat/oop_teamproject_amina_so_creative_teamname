#pragma once

#include <array>
#include <stdexcept>

namespace Constants {
    inline constexpr int DENOM_50000 = 50000;
    inline constexpr int DENOM_10000 = 10000;
    inline constexpr int DENOM_5000  = 5000;
    inline constexpr int DENOM_1000  = 1000;

    inline constexpr std::array<int, 4> DENOMINATIONS = {
        DENOM_50000, DENOM_10000, DENOM_5000, DENOM_1000
    };

    inline constexpr int MAX_BILLS_PER_DEPOSIT       = 50;
    inline constexpr int MAX_WITHDRAWALS_PER_SESSION = 3;
    inline constexpr int MAX_PASSWORD_ATTEMPTS       = 3;
    inline constexpr long long MAX_WITHDRAWAL_AMOUNT = 500000;
    inline constexpr long long MIN_CHECK_AMOUNT      = 100000;
    inline constexpr int SERIAL_NUMBER_LENGTH        = 6;
    inline constexpr int ACCOUNT_NUMBER_DIGITS       = 12;

    inline constexpr char SNAPSHOT_COMMAND = '/';
}

enum class LanguageMode {
    Unilingual,
    Bilingual
};

// Note: Alan also defined Language in Messages.h on his branch,
// we'll keep just one when merging everything into main.
enum class Language {
    English,
    Korean
};

struct BillCount {
    int count50000 = 0;
    int count10000 = 0;
    int count5000  = 0;
    int count1000  = 0;

    int get(int denom) const {
        switch (denom) {
            case Constants::DENOM_50000: return count50000;
            case Constants::DENOM_10000: return count10000;
            case Constants::DENOM_5000:  return count5000;
            case Constants::DENOM_1000:  return count1000;
            default: return 0;
        }
    }

    void set(int denom, int count) {
        if (count < 0) {
            throw std::invalid_argument("bill count cannot be negative");
        }
        switch (denom) {
            case Constants::DENOM_50000: count50000 = count; break;
            case Constants::DENOM_10000: count10000 = count; break;
            case Constants::DENOM_5000:  count5000  = count; break;
            case Constants::DENOM_1000:  count1000  = count; break;
            default: throw std::invalid_argument("unknown denomination");
        }
    }

    void add(int denom, int count) {
        set(denom, get(denom) + count);
    }

    int totalBills() const {
        return count50000 + count10000 + count5000 + count1000;
    }

    long long total() const {
        return static_cast<long long>(count50000) * Constants::DENOM_50000 +
               static_cast<long long>(count10000) * Constants::DENOM_10000 +
               static_cast<long long>(count5000)  * Constants::DENOM_5000 +
               static_cast<long long>(count1000)  * Constants::DENOM_1000;
    }
};
