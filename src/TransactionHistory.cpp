// Owner: Alan (Person D)
#include "TransactionHistory.h"
#include <iostream>
#include <fstream>
#include "Messages.h"

using namespace std;

TransactionHistory& TransactionHistory::global() {
    static TransactionHistory hist;
    return hist;
}

void TransactionHistory::startSession(const string& cardNumber) {
    SessionLog log;
    log.sessionNumber = (int)sessions.size() + 1;
    log.cardNumber = cardNumber;
    sessions.push_back(log);
}

void TransactionHistory::addTransaction(int id, const string& cardNumber,
    const string& type, long long amount, const string& details) {
    if(sessions.empty()) {
        startSession(cardNumber);
    }

    TransactionRecord rec;
    rec.id = id;
    rec.cardNumber = cardNumber;
    rec.type = type;
    rec.amount = amount;
    rec.details = details;
    sessions.back().transactions.push_back(rec);
}

bool TransactionHistory::isEmpty() const {
    for (int i = 0; i < (int)sessions.size(); i++) {
        if (!sessions[i].transactions.empty()) {
            return false;
        }
    }
    return true;
}

string TransactionHistory::format(Language lang) const {
    string result = "";
    int num = 0;

    for (int i = 0; i < (int)sessions.size(); i++) {
        // empty sessions are skipped
        if (sessions[i].transactions.empty())
            continue;

        num++;
        result += Messages::get("hist.session", lang) + to_string(num) + " (" + Messages::get("hist.card", lang)
            + sessions[i].cardNumber + ")\n";
        for (int j = 0; j < (int)sessions[i].transactions.size(); j++) {
            TransactionRecord tx = sessions[i].transactions[j];
            result += "  [ID " + to_string(tx.id) + "] " + tx.type
                + " | " + Messages::get("hist.card", lang) + tx.cardNumber
                + " | " + Messages::get("hist.amount", lang) + to_string(tx.amount) + Messages::get("tx.won_suffix", lang);
            if (!tx.details.empty()) {
                result += " | " + tx.details;
            }
            result += "\n";
        }
    }
    return result;
}

void TransactionHistory::print(Language lang) const {
    cout << format(lang);
}

bool TransactionHistory::writeToFile(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    file << format();
    return file.good();
}
