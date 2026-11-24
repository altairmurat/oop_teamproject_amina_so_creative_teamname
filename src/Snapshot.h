// Owner: Rabia (Person A)
#pragma once
#include <vector>

class ATM;

// REQ10.1: typing "/" at any prompt prints the cash of every ATM and the balance of every account.
// main() gives the ATM list to setATMs() once and passes &Snapshot::print to
// ConsoleUI::setSnapshotCallback (Person B's ConsoleUI calls it when it sees "/").
// banks are taken from Bank::getAllBanks(), so we don't need to store them here
class Snapshot {
private:
    static std::vector<ATM*> m_atms;

public:
    static void setATMs(const std::vector<ATM*>& atms);
    static void print();
};
