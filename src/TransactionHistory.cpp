#include "TransactionHistory.h"
#include <fstream>

using namespace std;

void TransactionHistory::startSession(const string& cardNumber) {
    SessionLog log;
    log.sessionNumber = static_cast<int>(sessions.size()) + 1;
    log.cardNumber = cardNumber;
    sessions.push_back(log);
}

void TransactionHistory::addTransaction(int id, const string& cardNumber,
    const string& type, long long amount, const string& details) {
    if (sessions.empty()) {
        startSession(cardNumber);
    }
    TransactionRecord rec{id, cardNumber, type, amount, details};
    sessions.back().transactions.push_back(rec);
}

bool TransactionHistory::isEmpty() const {
    for (size_t i = 0; i < sessions.size(); ++i) {
        if (!sessions[i].transactions.empty()) {
            return false;
        }
    }
    return true;
}

string TransactionHistory::format() const {
    string result = "";
    for (size_t i = 0; i < sessions.size(); ++i) {
        const SessionLog& s = sessions[i];
        // empty sessions are skipped
        if (s.transactions.empty())
            continue;

        result += "Session #" + to_string(s.sessionNumber) + " (Card: " + s.cardNumber + ")\n";
        for (size_t j = 0; j < s.transactions.size(); ++j) {
            const TransactionRecord& tx = s.transactions[j];
            result += "  [ID " + to_string(tx.id) + "] " + tx.type
                + " | Card: " + tx.cardNumber
                + " | Amount: " + to_string(tx.amount) + " KRW";
            if (!tx.details.empty()) {
                result += " | " + tx.details;
            }
            result += "\n";
        }
    }
    return result;
}

void TransactionHistory::print(ostream& out) const {
    out << format();
}

bool TransactionHistory::writeToFile(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    file << format();
    return file.good();
}
