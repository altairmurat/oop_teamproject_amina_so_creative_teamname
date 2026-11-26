// Owner: Alan (Person D)
#pragma once
#include <string>
#include <vector>
#include "Constants.h"

struct TransactionRecord {
    int id;
    std::string cardNumber;
    std::string type;
    long long amount;
    std::string details;
};

struct SessionLog {
    int sessionNumber;
    std::string cardNumber;
    std::vector<TransactionRecord> transactions;
};

// Global history of successful transactions grouped by session
class TransactionHistory {
public:
    static TransactionHistory& global();

    void startSession(const std::string& cardNumber);
    void addTransaction(int id, const std::string& cardNumber,
        const std::string& type, long long amount, const std::string& details);

    bool isEmpty() const;
    std::string format(Language lang = Language::English) const;
    void print(Language lang = Language::English) const;
    bool writeToFile(const std::string& filename) const;

private:
    std::vector<SessionLog> sessions;
};
