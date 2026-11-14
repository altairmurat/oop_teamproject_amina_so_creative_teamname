#include "AdminMenu.h"
#include "ATM.h"
#include "ConsoleUI.h"
#include "Messages.h"
#include <iostream>

using namespace std;

string AdminMenu::buildOutputFilename(const ATM& atm) {
    // формат названия файла взял как в примере, если в пдф по другому потом поправим
    return atm.getPrimaryBankName() + " ATM " + atm.getSerialNumber() + " transac_history.txt";
}

void AdminMenu::run(const ATM& atm, const TransactionHistory& history) {
    Language lang = atm.getLanguage();

    while (true) {
        int choice = ConsoleUI::readMenuChoice(Messages::get("admin_menu", lang), 1, 2);
        if (choice == 2)
            break;

        if (history.isEmpty()) {
            cout << Messages::get("history_empty", lang) << "\n";
            continue;
        }

        cout << "\n" << Messages::get("history_header", lang) << "\n";
        history.print(cout);

        string filename = buildOutputFilename(atm);
        if (history.writeToFile(filename)) {
            cout << Messages::get("history_saved", lang) << filename << "\n";
        }
    }
}
