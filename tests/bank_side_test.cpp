// Owner: Seunggyun (Person A)
// Test driver for the bank side. Needs only Card, Account, Bank and SystemLoader.
// Run from the project folder:   make test-bank
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "../src/Bank.h"
#include "../src/Card.h"
#include "../src/Exceptions.h"
#include "../src/SystemLoader.h"

using namespace std;

int failures = 0;

void check(bool condition, const string& name) {
    if (condition) {
        cout << "  [PASS] " << name << "\n";
    } else {
        cout << "  [FAIL] " << name << "\n";
        failures++;
    }
}

void deleteBanks(vector<Bank*>& banks) {
    for (size_t i = 0; i < banks.size(); i++) {
        delete banks[i];
    }
    banks.clear();
}

// Writes content to a temporary file and returns true if the loader rejects it.
bool loaderRejects(const string& content) {
    const string path = "tests/_tmp_bad_input.txt";
    ofstream out(path);
    out << content;
    out.close();

    vector<Bank*> banks;
    vector<ATMConfig> atms;
    bool rejected = false;
    try {
        SystemLoader::load(path, banks, atms);
    } catch (runtime_error& e) {
        cout << "         -> " << e.what() << "\n";
        rejected = true;
    }
    deleteBanks(banks); // banks created before the error must still be freed
    remove(path.c_str());
    return rejected;
}

int main() {
    cout << "1. Loading data/initial_conditions.txt\n";
    vector<Bank*> banks;
    vector<ATMConfig> atms;
    SystemLoader::load("data/initial_conditions.txt", banks, atms);
    check(banks.size() == 2 && atms.size() == 2, "2 banks and 2 ATMs loaded");
    check(atms[0].serialNumber == "000111" && !atms[0].isMultiBank && !atms[0].isBilingual,
          "ATM 1 is 000111, Single, Unilingual");
    check(atms[1].isMultiBank && atms[1].isBilingual && atms[1].bills[3] == 60,
          "ATM 2 is Multi, Bilingual, 60 x 1000");

    cout << "2. Finding accounts\n";
    Account* chris = Bank::findAccount("100-100-111111");
    Account* andor = Bank::findAccount("200200222222"); // without dashes
    check(chris != nullptr && chris->getOwnerName() == "Chris" && chris->getBalance() == 4000,
          "Chris found with 4000");
    check(andor != nullptr && andor->getBankName() == "IM", "Andor found without dashes, bank IM");
    check(Bank::findAccount("999-999-999999") == nullptr, "unknown account gives nullptr");

    cout << "3. Card and password checks (REQ3.3)\n";
    Bank* kakao = Bank::findBank("Kakao");
    check(kakao->verifyCard("1111-1111-1111"), "Kakao knows Chris's card");
    check(!kakao->verifyCard("2222-2222-2222"), "Kakao does not know Andor's card");
    check(kakao->verifyPassword("1111-1111-1111", "1234"), "correct password accepted");
    check(!kakao->verifyPassword("1111-1111-1111", "0000"), "wrong password rejected");
    check(Bank::isAdminCard("0000"), "default admin card 0000 registered");
    check(!Bank::isAdminCard("1111-1111-1111"), "user card is not an admin card");

    cout << "4. Card inheritance (virtual isAdmin through a Card reference)\n";
    Card userCard("1111-1111-1111", "Kakao");
    AdminCard adminCard("0000");
    const Card& asCard = adminCard; // upcasting
    check(!userCard.isAdmin(), "Card::isAdmin() is false");
    check(asCard.isAdmin(), "AdminCard seen as Card still says isAdmin() true");
    check(asCard.getBankName().empty(), "AdminCard has no bank");

    cout << "5. Deposit, withdraw, transfer\n";
    kakao->deposit(*chris, 4000);
    check(chris->getBalance() == 8000, "deposit 4000: Chris 4000 -> 8000");
    Bank::transfer(*andor, *chris, 1000, 3000);
    check(andor->getBalance() == 2000 && chris->getBalance() == 9000,
          "transfer 1000 + fee 3000: Andor 6000 -> 2000, Chris 8000 -> 9000");

    bool thrown = false;
    try {
        Bank::transfer(*andor, *chris, 1000, 2000); // needs 3000, Andor has 2000
    } catch (InsufficientFundsException& e) {
        thrown = true;
    }
    check(thrown && andor->getBalance() == 2000 && chris->getBalance() == 9000,
          "transfer over the balance throws and changes nothing");

    thrown = false;
    try {
        chris->withdraw(9001);
    } catch (InsufficientFundsException& e) {
        thrown = true;
    }
    check(thrown && chris->getBalance() == 9000, "withdraw 9001 from 9000 throws");

    chris->withdraw(9000);
    check(chris->getBalance() == 0, "withdraw the whole balance leaves 0");

    thrown = false;
    try {
        chris->deposit(0);
    } catch (InvalidInputException& e) {
        thrown = true;
    }
    check(thrown, "deposit 0 throws InvalidInputException");

    thrown = false;
    try {
        kakao->deposit(*andor, 1000); // Andor's account is in IM, not Kakao
    } catch (InvalidInputException& e) {
        thrown = true;
    }
    check(thrown && andor->getBalance() == 2000, "Kakao cannot deposit into an IM account");

    deleteBanks(banks);
    check(Bank::getAllBanks().empty(), "deleting the banks empties the bank list");

    cout << "6. Malformed files are rejected\n";
    check(loaderRejects(""), "empty file");
    check(loaderRejects("1 0 1\nKakao\nIM\n000111\nSingle\nUni\n1 1 1 1\n"), "ATM with unknown bank");
    check(loaderRejects("1 0 1\nKakao\nKakao\n00011\nSingle\nUni\n1 1 1 1\n"), "5-digit serial");
    check(loaderRejects("1 0 1\nKakao\nKakao\n000111\nTriple\nUni\n1 1 1 1\n"), "bad ATM type");
    check(loaderRejects("1 0 1\nKakao\nKakao\n000111\nSingle\nUni\n1 1 1\n"), "only 3 cash counts");
    check(loaderRejects("1 1 1\nKakao\nKakao\nChris\n100-100-11111\n4000\n1\n1\n"), "11-digit account");
    check(loaderRejects("1 1 1\nKakao\nKakao\nChris\n100-100-111111\n-5\n1\n1\n"), "negative funds");
    check(loaderRejects("2 0 1\nKakao\nKakao\nKakao\n000111\nSingle\nUni\n1 1 1 1\n"), "duplicate bank");
    check(loaderRejects("1 0 1\nKakao\nKakao\n000111\nSingle\n"), "file ends too early");
    check(Bank::getAllBanks().empty(), "no banks left over after the failed loads");

    if (failures == 0) {
        cout << "All bank-side tests passed.\n";
        return 0;
    }
    cout << failures << " test(s) failed.\n";
    return 1;
}
