// Owner: Altair (Person B)
#include "Session.h"
#include "ATM.h"
#include "Account.h"
#include "Bank.h"
#include "Card.h"
#include "ConsoleUI.h"
#include "DepositTransaction.h"
#include "Exceptions.h"
#include "FeePolicy.h"
#include "Messages.h"
#include "Transaction.h"
#include "WithdrawalTransaction.h"
#include <fstream>
#include <iostream>

namespace {

// this is to keep a global log of all successful transactions across sessions
// until Alan's TransactionHistory and AdminMenu are merged into main
std::vector<std::string>& globalHistoryLog() {
    static std::vector<std::string> log;
    return log;
}

// local cash transfer transaction until Alan merges CashTransferTransaction into main
class LocalCashTransfer : public Transaction {
public:
    using Transaction::Transaction;

    void execute() override {
        std::string destNum = ConsoleUI::readLine("Enter destination account number: ");
        Account* dest = Bank::findAccount(destNum);
        if (!dest) {
            throw InvalidInputException("Error: destination account not found.");
        }

        ConsoleUI::showMessage("Insert cash bills (includes KRW 1,000 fee):");
        BillCount inserted = ConsoleUI::readBills(Constants::MAX_BILLS_PER_DEPOSIT);
        long long totalInserted = inserted.total();
        long long transferFee = FeePolicy::cashTransferFee();

        if (totalInserted <= transferFee) {
            throw InvalidInputException("Error: inserted cash must be greater than the KRW 1,000 fee.");
        }

        amount = totalInserted - transferFee;
        fee = transferFee;
        destAccount = dest->getAccountNumber();

        Bank* destBank = Bank::findBank(dest->getBankName());
        if (!destBank) {
            throw ATMException("Error: destination bank not found.");
        }
        destBank->deposit(*dest, amount);
        transactionATM.addCash(inserted);

        ConsoleUI::showMessage("Fee (KRW): " + std::to_string(fee));
        ConsoleUI::showMessage("Cash transfer completed.");
        commit();
    }

    std::string type() const override { return "Cash Transfer"; }

    std::string describe() const override {
        return "Cash Transfer -> " + destAccount + " (" + std::to_string(amount) + " Won)";
    }

private:
    std::string destAccount;
};

// local account transfer transaction until Alan merges AccountTransferTransaction into main
class LocalAccountTransfer : public Transaction {
public:
    using Transaction::Transaction;

    void execute() override {
        std::string destNum = ConsoleUI::readLine("Enter destination account number: ");
        Account* dest = Bank::findAccount(destNum);
        if (!dest) {
            throw InvalidInputException("Error: destination account not found.");
        }
        if (dest == &transactionAccount) {
            throw InvalidInputException("Error: cannot transfer to the same account.");
        }

        long long transferAmount = ConsoleUI::readPositiveAmount("Enter transfer amount (KRW): ");
        bool srcPrimary = transactionATM.isPrimaryBank(transactionAccount.getBankName());
        bool dstPrimary = transactionATM.isPrimaryBank(dest->getBankName());
        long long transferFee = FeePolicy::accountTransferFee(srcPrimary, dstPrimary);

        if (!transactionAccount.canWithdraw(transferAmount + transferFee)) {
            throw InsufficientFundsException("Error: insufficient funds in the account (amount + fee).");
        }

        amount = transferAmount;
        fee = transferFee;
        destAccount = dest->getAccountNumber();

        Bank::transfer(transactionAccount, *dest, amount, fee);

        ConsoleUI::showMessage("Fee (KRW): " + std::to_string(fee));
        ConsoleUI::showMessage("Account transfer completed.");
        commit();
    }

    std::string type() const override { return "Account Transfer"; }

    std::string describe() const override {
        return "Account Transfer " + transactionAccount.getAccountNumber() +
               " -> " + destAccount + " (" + std::to_string(amount) + " Won)";
    }

private:
    std::string destAccount;
};

} // namespace

Session::Session(ATM& atm)
    : atm(atm) {}

void Session::run(const Card& card, Bank& issuingBank) {
    active = true;
    authorized = false;
    wrongPasswordAttempts = 0;
    withdrawalCount = 0;
    completedTransactions.clear();

    try {
        // 1. select language if this ATM is bilingual (REQ1.3, REQ8.1)
        Language chosen = ConsoleUI::promptLanguage(atm);
        atm.setLanguage(chosen);

        // 2. check if this ATM accepts the inserted card (REQ3.1, REQ3.2)
        if (!verifyCard(card)) {
            ConsoleUI::showMessage("Invalid card for this ATM. The card is returned.");
            endSession();
            return;
        }

        // 3. route admin card directly to the admin menu (REQ1.9, REQ7.1)
        if (card.isAdmin()) {
            runAdminMenu();
            endSession();
            return;
        }

        // 4. verify password with the issuing bank, max 3 wrong tries (REQ3.3, REQ3.4, REQ3.5)
        if (!authorizeUser(card, issuingBank)) {
            ConsoleUI::showMessage("Too many wrong password attempts. Session ended and card returned.");
            endSession();
            return;
        }

        Account* account = issuingBank.getAccountByCard(card.getCardNumber());
        if (!account) {
            ConsoleUI::showMessage("Account not found for this card. Session ended.");
            endSession();
            return;
        }

        // 5. run the transaction loop until user cancels or an exception occurs
        runMenuLoop(*account, card);
    } catch (const std::exception&) {
        if (!std::cin) {
            // input stream ended (e.g. test script EOF), still print summary before exiting
            printSummary();
            endSession();
            throw;
        }
        throw;
    }

    // 6. print session summary at the end if at least one transaction succeeded (REQ2.3)
    printSummary();
    ConsoleUI::showMessage("Session ended. Please take your card.");
    endSession();
}

bool Session::verifyCard(const Card& card) {
    return atm.isCardValid(card);
}

bool Session::authorizeUser(const Card& card, Bank& issuingBank) {
    while (wrongPasswordAttempts < Constants::MAX_PASSWORD_ATTEMPTS) {
        std::string pin = ConsoleUI::readLine("Enter password: ");
        if (issuingBank.verifyPassword(card.getCardNumber(), pin)) {
            authorized = true;
            return true;
        }
        ++wrongPasswordAttempts;
        ConsoleUI::showMessage("Wrong password. Please try again.");
    }
    return false;
}

void Session::runMenuLoop(Account& account, const Card& card) {
    while (active) {
        int choice = ConsoleUI::readMenuChoice(
            "\n1. Deposit\n2. Withdrawal\n3. Transfer\n4. Cancel\nSelect: ",
            1, 4
        );
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
                    "\n1. Cash Transfer\n2. Account Transfer\nSelect: ",
                    1, 2
                );
                if (transferType == 1) {
                    tx = std::make_shared<LocalCashTransfer>(atm, account);
                } else {
                    tx = std::make_shared<LocalAccountTransfer>(atm, account);
                }
            }

            if (tx) {
                tx->execute();
                if (choice == 2) {
                    recordWithdrawal();
                }
                addCompletedTransaction(tx);
                account.addHistory(tx->describe());
                globalHistoryLog().push_back(
                    "[ID " + std::to_string(tx->transactionID()) + "] " +
                    "Card: " + card.getCardNumber() + " | " +
                    tx->type() + " | " + tx->describe() +
                    " | Fee: " + std::to_string(tx->transactionFee()) + " Won"
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

void Session::runAdminMenu() {
    while (true) {
        int choice = ConsoleUI::readMenuChoice(
            "\n[Admin Menu]\n1. Transaction History\n2. Exit\nSelect: ",
            1, 2
        );
        if (choice == 2) {
            break;
        }

        if (globalHistoryLog().empty()) {
            ConsoleUI::showMessage("No transactions recorded yet.");
            continue;
        }

        ConsoleUI::showMessage("\n=== Transaction History ===");
        for (const std::string& entry : globalHistoryLog()) {
            ConsoleUI::showMessage(entry);
        }

        std::string filename = atm.getPrimaryBankName() + " ATM " +
                               atm.getSerialNumber() + " transac_history.txt";
        std::ofstream out(filename);
        if (out) {
            for (const std::string& entry : globalHistoryLog()) {
                out << entry << "\n";
            }
            ConsoleUI::showMessage("Transaction history saved to file: " + filename);
        }
    }
}

bool Session::canWithdraw() const {
    return withdrawalCount < Constants::MAX_WITHDRAWALS_PER_SESSION;
}

void Session::recordWithdrawal() {
    if (!canWithdraw()) {
        throw LimitExceededException("Error: maximum number of withdrawals per session (3) reached.");
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
    ConsoleUI::showMessage("\n=== Session Summary ===");
    for (const auto& tx : completedTransactions) {
        ConsoleUI::showMessage(
            "[ID " + std::to_string(tx->transactionID()) + "] " +
            tx->type() + " - " + tx->describe() +
            " (Fee: " + std::to_string(tx->transactionFee()) + " Won)"
        );
    }
    ConsoleUI::showMessage("=======================");
}

void Session::endSession() {
    active = false;
    authorized = false;
}
