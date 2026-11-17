#include "Session.h"
#include "ATM.h"
#include "ConsoleUI.h"
#include "Transaction.h"
// #include "Bank.h"
// #include "Card.h"
// #include "Account.h"
// #include "AdminMenu.h"
// #include "Exceptions.h"

Session::Session(ATM& atm)
    : atm(atm) {}

void Session::run(const Card& card, Bank& issuingBank) {
    active = true;
    authorized = false;
    wrongPasswordAttempts = 0;
    withdrawalCount = 0;
    completedTransactions.clear();

    Language chosen = ConsoleUI::promptLanguage(atm);
    atm.setLanguage(chosen);

    if (!verifyCard(card)) {
        ConsoleUI::showMessage("This card cannot be used on this ATM.");
        endSession();
        return;
    }

    // admin card check will go here once Card::isAdmin() and AdminMenu are merged

    if (!authorizeUser(card, issuingBank)) {
        ConsoleUI::showMessage("Too many wrong password attempts. Session ended.");
        endSession();
        return;
    }

    // once Bank::findAccountByCard / Account are ready, call runMenuLoop(account)
    (void)issuingBank;
    ConsoleUI::showMessage("Authorized.");

    printSummary();
    endSession();
}

bool Session::verifyCard(const Card& card) {
    return atm.isCardValid(card);
}

bool Session::authorizeUser(const Card& card, Bank& issuingBank) {
    (void)card;
    (void)issuingBank;

    while (wrongPasswordAttempts < Constants::MAX_PASSWORD_ATTEMPTS) {
        std::string pin = ConsoleUI::readLine("Enter password: ");
        // replace with issuingBank.verifyPassword(card, pin) once Bank.h is ready
        if (!pin.empty()) {
            authorized = true;
            return true;
        }
        ++wrongPasswordAttempts;
        ConsoleUI::showMessage("Wrong password. Please try again.");
    }
    return false;
}

void Session::runMenuLoop(Account& account) {
    (void)account;
    while (active) {
        int choice = ConsoleUI::readMenuChoice(
            "\n1. Deposit\n2. Withdrawal\n3. Transfer\n4. Cancel\nSelect: ",
            1, 4
        );
        if (choice == 4) {
            break;
        }
        // hook up DepositTransaction, WithdrawalTransaction, CashTransfer, AccountTransfer
        // and catch ATMException to end session on any error (REQ 9.1)
        ConsoleUI::showMessage("Transaction handlers not wired yet.");
    }
}

bool Session::canWithdraw() const {
    return withdrawalCount < Constants::MAX_WITHDRAWALS_PER_SESSION;
}

void Session::recordWithdrawal() {
    if (!canWithdraw()) {
        // swap to LimitExceededException once Exceptions.h is merged
        throw std::runtime_error("Transaction limit exceeded.");
    }
    ++withdrawalCount;
}

void Session::addCompletedTransaction(const std::shared_ptr<Transaction>& tx) {
    if (tx) {
        completedTransactions.push_back(tx);
    }
}

void Session::printSummary() const {
    if (completedTransactions.empty()) {
        return;
    }
    ConsoleUI::showMessage("=== Session Summary ===");
    for (const auto& tx : completedTransactions) {
        ConsoleUI::showMessage(
            "[" + std::to_string(tx->transactionID()) + "] " +
            tx->type() + " - " + tx->describe()
        );
    }
}

void Session::endSession() {
    active = false;
    authorized = false;
}
