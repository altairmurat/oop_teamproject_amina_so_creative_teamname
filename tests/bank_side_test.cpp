// Owner: Seunggyun (Person A)
// Small driver for the bank side. Builds without Person B/C/D code:
//   make test-bank
#include <cassert>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include "../src/Bank.h"
#include "../src/SystemLoader.h"

static bool loadFails(const std::string& content) {
    const char* tmp = "tests/_tmp_bad_input.txt";
    { std::ofstream(tmp) << content; }
    try { SystemLoader::load(tmp); } catch (const std::runtime_error& e) {
        std::cout << "  rejected as expected -> " << e.what() << "\n";
        std::remove(tmp);
        return true;
    }
    std::remove(tmp);
    return false;
}

int main() {
    {
        SystemData sys = SystemLoader::load("data/initial_conditions.txt");
        assert(sys.banks.size() == 2 && sys.atms.size() == 2);
        assert(sys.atms[0].serialNumber == "000111" && !sys.atms[0].multiBank);
        assert(sys.atms[1].bilingual && sys.atms[1].bills[3] == 60);

        Account* chris = Bank::findAccount("100-100-111111");
        Account* andor = Bank::findAccount("200200222222"); // without dashes works too
        assert(chris && andor && chris->getBalance() == 4000);
        assert(chris->getBankName() == "Kakao" && andor->getBankName() == "IM");

        Bank* kakao = Bank::findBank("Kakao");
        assert(kakao->verifyCard("1111-1111-1111") && !kakao->verifyCard("2222-2222-2222"));
        assert(kakao->verifyPassword("1111-1111-1111", "1234"));
        assert(!kakao->verifyPassword("1111-1111-1111", "0000"));

        auto card = Bank::lookupCard("1111-1111-1111");
        assert(card && !card->isAdmin() && card->getBankName() == "Kakao");
        auto admin = Bank::lookupCard("0000");
        assert(admin && admin->isAdmin());
        assert(!Bank::lookupCard("9999"));

        Bank::transfer(*andor, *chris, 1000, 3000); // 6000 -> 2000, 4000 -> 5000
        assert(andor->getBalance() == 2000 && chris->getBalance() == 5000);
        bool threw = false;
        try { Bank::transfer(*andor, *chris, 1000, 2000); } catch (const std::runtime_error&) { threw = true; }
        assert(threw && andor->getBalance() == 2000); // nothing changed

        chris->deposit(4000);
        assert(chris->getBalance() == 9000);
        std::cout << "Valid file: OK\n";
    } // banks destroyed here, registry is empty again
    assert(Bank::allBanks().empty());

    std::cout << "Malformed files:\n";
    assert(loadFails(""));
    assert(loadFails("1 0 1\nKakao\nIM\n000111\nSingle\nUni\n1 1 1 1\n"));        // unknown bank
    assert(loadFails("1 0 1\nKakao\nKakao\n00011\nSingle\nUni\n1 1 1 1\n"));      // 5-digit serial
    assert(loadFails("1 0 1\nKakao\nKakao\n000111\nTriple\nUni\n1 1 1 1\n"));     // bad type
    assert(loadFails("1 0 1\nKakao\nKakao\n000111\nSingle\nUni\n1 1 1\n"));       // 3 counts
    assert(loadFails("1 1 1\nKakao\nKakao\nChris\n100-100-11111\n4000\n1\n1\n")); // 11 digits
    assert(loadFails("1 1 1\nKakao\nKakao\nChris\n100-100-111111\n-5\n1\n1\n"));  // negative
    assert(loadFails("2 0 1\nKakao\nKakao\nKakao\n000111\nSingle\nUni\n1 1 1 1\n")); // dup bank
    assert(loadFails("1 0 1\nKakao\nKakao\n000111\nSingle\n"));                  // truncated

    std::cout << "All bank-side tests passed.\n";
    return 0;
}
