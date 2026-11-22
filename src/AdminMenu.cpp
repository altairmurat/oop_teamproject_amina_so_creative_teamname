// Owner: Alan (Person D)
#include "AdminMenu.h"
#include <iostream>
#include <string>
#include "ATM.h"
#include "ConsoleUI.h"
#include "Messages.h"

using namespace std;

void AdminMenu::run(const ATM& atm, const TransactionHistory& history) {
    Language lang = atm.getLanguage();

    while (true) {
        int choice = ConsoleUI::readMenuChoice(Messages::get("admin_menu", lang), 1, 2);
        if(choice == 2)
            break;

        if (history.isEmpty()) {
            ConsoleUI::showMessage(Messages::get("history_empty", lang));
            continue;
        }

        cout << "\n" << Messages::get("history_header", lang) << "\n";
        history.print();

        string filename = atm.getPrimaryBankName() + " ATM " + atm.getSerialNumber() + " transac_history.txt";
        if (history.writeToFile(filename)) {
            ConsoleUI::showMessage(Messages::get("history_saved", lang) + filename);
        }
    }
}
