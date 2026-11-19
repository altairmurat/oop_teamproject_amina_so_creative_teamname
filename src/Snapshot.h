// Owner: Seunggyun (Person A)
#pragma once
#include <iostream>
#include <vector>

class ATM;

// REQ10.1: typing "/" at any prompt prints all ATM cash and all account balances.
// main() hooks Snapshot::print() into ConsoleUI::setSnapshotCallback, so
// ConsoleUI (Person B) prints it whenever it sees "/".
// main() registers the ATMs once; banks are found through Bank::allBanks().
class Snapshot {
public:
    static void setATMs(std::vector<const ATM*> atmList);
    static void print(std::ostream& os = std::cout);

private:
    static std::vector<const ATM*>& atms();
};
