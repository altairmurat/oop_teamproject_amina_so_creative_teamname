#pragma once
#include <string>
#include <vector>

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

// global history where only succesful transactions is recorded and grouped by session
class TransactionHistory {
public:
    // добавил global() чтобы в Session.cpp было проще подключать вместо globalHistoryLog
    static TransactionHistory& global();

    void startSession(const std::string& cardNumber);
    void addTransaction(int id, const std::string& cardNumber,
        const std::string& type, long long amount, const std::string& details);

    bool isEmpty() const;
    std::string format() const;
    void print() const;
    bool writeToFile(const std::string& filename) const;

private:
    std::vector<SessionLog> sessions;
};
