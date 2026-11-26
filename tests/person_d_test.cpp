// Owner: Alan (Person D)
// Tests for Person D modules (Messages, Exceptions, Transfers, TransactionHistory, AdminMenu)
// Build from project root:
//   g++ -std=c++17 -Wall -Wextra -Isrc tests/person_d_test.cpp $(ls src/*.cpp | grep -v main.cpp) -o tests/_tmp_person_d_test && ./tests/_tmp_person_d_test
#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "ATM.h"
#include "Account.h"
#include "AccountTransferTransaction.h"
#include "AdminMenu.h"
#include "Bank.h"
#include "Card.h"
#include "CashInventory.h"
#include "CashTransferTransaction.h"
#include "ConsoleUI.h"
#include "Constants.h"
#include "Exceptions.h"
#include "Messages.h"
#include "MultiBankATM.h"
#include "Session.h"
#include "SingleBankATM.h"
#include "TransactionHistory.h"

using namespace std;

int failed = 0;

void check(bool cond, const string& label) {
    if (cond) {
        cout << "  [PASS] " << label << "\n";
    } else {
        cout << "  [FAIL] " << label << "\n";
        failed++;
    }
}

// helper to feed fake stdin to interactive classes during tests
struct StdinRedirect {
    streambuf* oldCin;
    istringstream input;

    StdinRedirect(const string& data) : oldCin(cin.rdbuf()), input(data) {
        cin.rdbuf(input.rdbuf());
    }
    ~StdinRedirect() {
        cin.rdbuf(oldCin);
    }
};

void testMessages() {
    cout << "1. Messages (English & Korean keys)\n";
    vector<string> keys = {
        "select_language", "insert_card", "enter_password", "wrong_password",
        "auth_failed", "invalid_card", "session_end", "session_summary",
        "main_menu", "choose_transfer_type", "enter_dest_account", "enter_amount",
        "insert_cash_bills", "invalid_input", "account_not_found", "bank_not_found",
        "same_account", "cash_transfer_too_small", "insufficient_funds",
        "insufficient_cash", "limit_exceeded", "fee_notice", "transfer_success",
        "admin_menu", "history_header", "history_empty", "history_saved",
        "prompt.bills_of", "err.too_many_bills", "deposit.choose_type",
        "deposit.enter_check", "deposit.fee_label", "err.check_too_small",
        "err.no_cash", "err.fee_mismatch", "deposit.success",
        "withdraw.enter_amount", "err.withdraw_count", "err.withdraw_max",
        "err.amount_unit", "err.no_funds", "err.no_cash_atm",
        "err.too_many_dispense", "withdraw.dispensing", "withdraw.fee_label",
        "withdraw.success", "tx.cash_transfer", "tx.account_transfer",
        "transfer.amount", "transfer.confirm", "transfer.cancelled",
        "summary_card", "summary_account", "hist.session", "hist.card",
        "hist.amount"
    };

    bool allEn = true;
    bool allKr = true;
    bool allDiff = true;
    for (int i = 0; i < (int)keys.size(); i++) {
        string en = Messages::get(keys[i], Language::English);
        string kr = Messages::get(keys[i], Language::Korean);
        if (en.empty() || en == keys[i]) allEn = false;
        if (kr.empty() || kr == keys[i]) allKr = false;
        if (en == kr) allDiff = false;
    }

    check(allEn, "all keys have English text");
    check(allKr, "all keys have Korean text");
    check(allDiff, "English and Korean strings differ for every key");
    check(Messages::get("missing_test_key", Language::English) == "missing_test_key",
          "unknown key falls back to key name");
}

void testExceptions() {
    cout << "2. Exception hierarchy\n";
    int caught = 0;
    try { throw InvalidCardException(); } catch (const ATMException&) { caught++; }
    try { throw WrongPasswordException(); } catch (const ATMException&) { caught++; }
    try { throw InsufficientFundsException(); } catch (const ATMException&) { caught++; }
    try { throw InsufficientCashException(); } catch (const ATMException&) { caught++; }
    try { throw LimitExceededException(); } catch (const ATMException&) { caught++; }
    try { throw InvalidInputException(); } catch (const ATMException&) { caught++; }
    check(caught == 6, "all 6 exception types inherit from ATMException");
}

void testTransfers() {
    cout << "3. CashTransferTransaction & AccountTransferTransaction\n";
    Bank* kakao = new Bank("Kakao");
    Bank* im = new Bank("IM");
    Bank* woori = new Bank("Woori");

    kakao->openAccount("Chris", "100-100-111111", 50000, "1111-1111-1111", "1234");
    kakao->openAccount("Dana",  "100-100-333333", 10000, "3333-3333-3333", "1111");
    im->openAccount("Andor",    "200-200-222222", 50000, "2222-2222-2222", "4321");
    woori->openAccount("Min",   "300-300-444444", 50000, "4444-4444-4444", "9999");

    Account* chris = Bank::findAccount("100-100-111111");
    Account* dana  = Bank::findAccount("100-100-333333");
    Account* andor = Bank::findAccount("200-200-222222");
    Account* minAcc = Bank::findAccount("300-300-444444");

    CashInventory inv(2, 2, 2, 2);
    MultiBankATM atm("000222", "Kakao", LanguageMode::Bilingual, inv);
    long long startCash = atm.getInventory().getTotalCash();

    // cash transfer: insert 1 x 10,000 -> fee 1,000, dest gets 9,000, ATM gets +10,000
    {
        StdinRedirect in("200-200-222222\n0\n1\n0\n0\n1\n");
        CashTransferTransaction tx(atm, *chris);
        tx.execute();
        check(tx.transactionID() > 0, "cash transfer got a transaction ID");
        check(tx.transactionFee() == 1000 && tx.transactionAmount() == 9000,
              "cash transfer amount 9000 and fee 1000");
        check(andor->getBalance() == 59000, "destination balance increased by 9000");
        check(atm.getInventory().getTotalCash() == startCash + 10000,
              "ATM inventory received all inserted bills");
    }

    // cash transfer <= 1000 won must fail and change nothing
    {
        StdinRedirect in("200-200-222222\n0\n0\n0\n1\n");
        CashTransferTransaction tx(atm, *chris);
        bool threw = false;
        try {
            tx.execute();
        } catch (const InvalidInputException&) {
            threw = true;
        }
        check(threw && tx.transactionID() == 0 && andor->getBalance() == 59000,
              "cash transfer of 1000 KRW rejected without changing state");
    }

    // cancelling cash transfer at confirmation leaves balance and ATM cash unchanged
    {
        StdinRedirect in("200-200-222222\n0\n1\n0\n0\n2\n");
        CashTransferTransaction tx(atm, *chris);
        bool threw = false;
        try {
            tx.execute();
        } catch (const InvalidInputException&) {
            threw = true;
        }
        check(threw && tx.transactionID() == 0 && andor->getBalance() == 59000
              && atm.getInventory().getTotalCash() == startCash + 10000,
              "cancelled cash transfer returns cash without changing state");
    }

    // account transfer primary -> primary (Kakao -> Kakao on Kakao ATM): fee 2000
    {
        StdinRedirect in("100-100-333333\n5000\n");
        AccountTransferTransaction tx(atm, *chris);
        tx.execute();
        check(tx.transactionFee() == 2000 && chris->getBalance() == 43000 && dana->getBalance() == 15000,
              "account transfer primary->primary charges 2000 fee");
    }

    // account transfer primary -> non-primary (Kakao -> IM on Kakao ATM): fee 3000
    {
        StdinRedirect in("200-200-222222\n5000\n");
        AccountTransferTransaction tx(atm, *chris);
        tx.execute();
        check(tx.transactionFee() == 3000 && chris->getBalance() == 35000 && andor->getBalance() == 64000,
              "account transfer primary->non-primary charges 3000 fee");
    }

    // account transfer non-primary -> non-primary (IM -> Woori on Kakao ATM): fee 5000
    {
        StdinRedirect in("300-300-444444\n5000\n");
        AccountTransferTransaction tx(atm, *andor);
        tx.execute();
        check(tx.transactionFee() == 5000 && andor->getBalance() == 54000 && minAcc->getBalance() == 55000,
              "account transfer non-primary->non-primary charges 5000 fee");
    }

    // same account transfer must fail
    {
        StdinRedirect in("100-100-111111\n");
        AccountTransferTransaction tx(atm, *chris);
        bool threw = false;
        try {
            tx.execute();
        } catch (const InvalidInputException&) {
            threw = true;
        }
        check(threw && tx.transactionID() == 0, "transfer to same account rejected");
    }

    // insufficient funds (balance + fee check)
    {
        StdinRedirect in("200-200-222222\n34000\n"); // chris has 35000, needs 34000 + 3000 = 37000
        AccountTransferTransaction tx(atm, *chris);
        bool threw = false;
        try {
            tx.execute();
        } catch (const InsufficientFundsException&) {
            threw = true;
        }
        check(threw && chris->getBalance() == 35000 && andor->getBalance() == 54000,
              "account transfer rejected when balance < amount + fee");
    }

    delete woori;
    delete im;
    delete kakao;
}

void testHistoryAndAdminMenu() {
    cout << "4. TransactionHistory session grouping & AdminMenu\n";
    TransactionHistory hist;
    check(hist.isEmpty(), "new history starts empty");

    // empty session should be ignored in format() and isEmpty()
    hist.startSession("9999-9999-9999");
    check(hist.isEmpty(), "history with only an empty session still counts as empty");

    hist.startSession("1111-1111-1111");
    hist.addTransaction(1, "1111-1111-1111", "Cash Transfer", 9000, "Cash Transfer -> 200-200-222222 (9000 Won)");
    hist.addTransaction(2, "1111-1111-1111", "Account Transfer", 5000, "Account Transfer 100-100-111111 -> 200-200-222222 (5000 Won)");

    // another empty session in the middle
    hist.startSession("8888-8888-8888");

    hist.startSession("2222-2222-2222");
    hist.addTransaction(3, "2222-2222-2222", "Cash Deposit", 10000, "10000Won added to 200-200-222222");

    string formatted = hist.format();
    string formattedKr = hist.format(Language::Korean);
    check(!hist.isEmpty(), "history is not empty after adding transactions");
    check(formatted.find("Session #1 (Card: 1111-1111-1111)") != string::npos,
          "session #1 header present");
    check(formattedKr.find("세션 #1 (카드: 1111-1111-1111)") != string::npos,
          "Korean session #1 header present");
    check(formatted.find("Session #2 (Card: 2222-2222-2222)") != string::npos,
          "empty sessions skipped so second non-empty session is #2");
    check(formatted.find("9999-9999-9999") == string::npos && formatted.find("8888-8888-8888") == string::npos,
          "empty sessions do not appear in output");

    // AdminMenu should print and write file in Korean mode too
    CashInventory inv(1, 1, 1, 1);
    MultiBankATM atm("000999", "TestBank", LanguageMode::Bilingual, inv);
    atm.setLanguage(Language::Korean);
    ConsoleUI::setLanguage(Language::Korean);

    string expectedFile = "TestBank ATM 000999 transac_history.txt";
    remove(expectedFile.c_str());
    {
        StdinRedirect in("1\n2\n");
        AdminMenu::run(atm, hist);
    }

    ifstream saved(expectedFile);
    stringstream buf;
    buf << saved.rdbuf();
    string fileContent = buf.str();
    saved.close();
    remove(expectedFile.c_str());

    check(fileContent == formatted, "AdminMenu wrote exact formatted history to file");
    ConsoleUI::setLanguage(Language::English);
}

int main() {
    testMessages();
    testExceptions();
    testTransfers();
    testHistoryAndAdminMenu();

    if (failed == 0) {
        cout << "All Person D tests passed.\n";
        return 0;
    }
    cout << failed << " test(s) failed.\n";
    return 1;
}
