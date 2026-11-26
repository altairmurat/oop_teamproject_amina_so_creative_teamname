// Owner: Altair (Person B)
#include "Session.h"
#include "ATM.h"
#include "Account.h"
#include "AccountTransferTransaction.h"
#include "AdminMenu.h"
#include "Bank.h"
#include "Card.h"
#include "CashTransferTransaction.h"
#include "ConsoleUI.h"
#include "DepositTransaction.h"
#include "Exceptions.h"
#include "Messages.h"
#include "Transaction.h"
#include "TransactionHistory.h"
#include "WithdrawalTransaction.h"
#include <iostream>

Session::Session(ATM& atm)
    : atm(atm) {}

void Session::run(const Card& card, Bank& issuingBank) {
    active = true;
    authorized = false;
    wrongPasswordAttempts = 0;
    withdrawalCount = 0;
    completedTransactions.clear();
    summaryCard.clear();
    summaryAccount.clear();

    try {
        // 1. select language if this ATM is bilingual (REQ1.3, REQ8.1)
        Language chosen = ConsoleUI::promptLanguage(atm);
        atm.setLanguage(chosen);
        ConsoleUI::setLanguage(chosen);
        Language lang = atm.getLanguage();

        // 2. check if this ATM accepts the inserted card (REQ3.1, REQ3.2)
        if (!verifyCard(card)) {
            ConsoleUI::showMessage(Messages::get("invalid_card", lang));
            ConsoleUI::showMessage(Messages::get("session_end", lang));
            endSession();
            return;
        }

        // 3. route admin card directly to Alan's AdminMenu (REQ1.9, REQ7.1)
        if (card.isAdmin()) {
            AdminMenu::run(atm, TransactionHistory::global());
            ConsoleUI::showMessage(Messages::get("session_end", lang));
            endSession();
            return;
        }

        // 4. verify password with the issuing bank, max 3 wrong tries (REQ3.3, REQ3.4, REQ3.5)
        if (!authorizeUser(card, issuingBank)) {
            ConsoleUI::showMessage(Messages::get("auth_failed", lang));
            ConsoleUI::showMessage(Messages::get("session_end", lang));  // card is returned (REQ3.5)
            endSession();
            return;
        }

        Account* account = issuingBank.getAccountByCard(card.getCardNumber());
        if (!account) {
            ConsoleUI::showMessage(Messages::get("account_not_found", lang));
            ConsoleUI::showMessage(Messages::get("session_end", lang));
            endSession();
            return;
        }

        summaryCard = card.getCardNumber();
        summaryAccount = account->getAccountNumber();

        // start a new session entry in the global transaction history
        TransactionHistory::global().startSession(card.getCardNumber());

        // 5. run the transaction loop until user cancels or an exception occurs
        runMenuLoop(*account, card);
    } catch (const std::exception&) {
        if (!std::cin) {
            // input stream ended (e.g. piped test script), still print summary before exiting
            printSummary();
            endSession();
            throw;
        }
        endSession();
        throw;
    }

    // 6. print session summary at the end if at least one transaction succeeded (REQ2.3)
    printSummary();
    ConsoleUI::showMessage(Messages::get("session_end", atm.getLanguage()));
    endSession();
}

bool Session::verifyCard(const Card& card) {
    return atm.isCardValid(card);
}

bool Session::authorizeUser(const Card& card, Bank& issuingBank) {
    Language lang = atm.getLanguage();
    while (wrongPasswordAttempts < Constants::MAX_PASSWORD_ATTEMPTS) {
        std::string pin = ConsoleUI::readLine(Messages::get("enter_password", lang));
        if (issuingBank.verifyPassword(card.getCardNumber(), pin)) {
            authorized = true;
            return true;
        }
        ++wrongPasswordAttempts;
        ConsoleUI::showMessage(Messages::get("wrong_password", lang));
    }
    return false;
}

void Session::runMenuLoop(Account& account, const Card& card) {
    Language lang = atm.getLanguage();

    while (active) {
        int choice = ConsoleUI::readMenuChoice(Messages::get("main_menu", lang), 1, 4);
        if (choice == 4) {
            break;
        }

        try {
            std::shared_ptr<Transaction> tx;

            if (choice == 1) {
                tx = std::make_shared<DepositTransaction>(atm, account);
            } else if (choice == 2) {
                tx = std::make_shared<WithdrawalTransaction>(atm, account, getWithdrawalCount());
            } else if (choice == 3) {
                int transferType = ConsoleUI::readMenuChoice(
                    Messages::get("choose_transfer_type", lang), 1, 2
                );
                if (transferType == 1) {
                    tx = std::make_shared<CashTransferTransaction>(atm, account);
                } else {
                    tx = std::make_shared<AccountTransferTransaction>(atm, account);
                }
            }

            if (tx) {
                tx->execute();
                if (choice == 2) {
                    recordWithdrawal();
                }
                addCompletedTransaction(tx);
                account.addHistory(tx->describe());
                TransactionHistory::global().addTransaction(
                    tx->transactionID(),
                    card.getCardNumber(),
                    tx->type(),
                    tx->transactionAmount(),
                    tx->describe()
                );
            }
        } catch (const ATMException& e) {
            // REQ9.1: any exception prints a message and immediately ends the session
            ConsoleUI::showMessage(e.what());
            endSession();
            break;
        }
    }
}

bool Session::canWithdraw() const {
    return withdrawalCount < Constants::MAX_WITHDRAWALS_PER_SESSION;
}

void Session::recordWithdrawal() {
    if (!canWithdraw()) {
        throw LimitExceededException(Messages::get("err.withdraw_count", atm.getLanguage()));
    }
    ++withdrawalCount;
}

void Session::addCompletedTransaction(const std::shared_ptr<Transaction>& tx) {
    if (tx) {
        completedTransactions.push_back(tx);
    }
}

void Session::printSummary() const {
    // skip summary if no transactions succeeded in this session (REQ2.3)
    if (completedTransactions.empty()) {
        return;
    }
    Language lang = atm.getLanguage();
    ConsoleUI::showMessage("\n" + Messages::get("session_summary", lang));
    ConsoleUI::showMessage(Messages::get("summary_card", lang) + summaryCard +
                           Messages::get("summary_account", lang) + summaryAccount);
    for (const auto& tx : completedTransactions) {
        ConsoleUI::showMessage(
            "[ID " + std::to_string(tx->transactionID()) + "] " +
            tx->type() + " - " + tx->describe() + " | " +
            Messages::get("fee_notice", lang) + std::to_string(tx->transactionFee())
        );
    }
}

void Session::endSession() {
    active = false;
    authorized = false;
    // reset ConsoleUI language to English for the outer ATM selection screen
    ConsoleUI::setLanguage(Language::English);
}
