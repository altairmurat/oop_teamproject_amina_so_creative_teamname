// Owner: Altair (Person B)
#pragma once
#include <string>
#include "Constants.h"
#include "CashInventory.h"

class Card;

// abstract base class for SingleBankATM and MultiBankATM
class ATM {
public:
    ATM(const std::string& serialNumber,
        const std::string& primaryBankName,
        LanguageMode languageMode,
        const CashInventory& initialInventory);

    virtual ~ATM() = default;

    // this is to let each ATM type decide which cards it accepts (polymorphism)
    virtual bool isCardValid(const Card& card) const = 0;
    virtual std::string getType() const = 0;

    const std::string& getSerialNumber() const { return serialNumber; }
    const std::string& getPrimaryBankName() const { return primaryBankName; }
    bool isPrimaryBank(const std::string& bankName) const;

    LanguageMode getLanguageMode() const { return languageMode; }
    Language getLanguage() const { return currentLanguage; }
    void setLanguage(Language lang);

    CashInventory& getInventory() { return inventory; }
    const CashInventory& getInventory() const { return inventory; }

    long long getTotalCash() const { return inventory.getTotalCash(); }
    void addCash(const BillCount& bills) { inventory.addBills(bills); }
    bool canDispenseCash(long long amount) const { return inventory.canDispense(amount); }
    BillCount dispenseCash(long long amount) { return inventory.dispense(amount); }

protected:
    std::string serialNumber;
    std::string primaryBankName;
    LanguageMode languageMode;
    Language currentLanguage = Language::English;
    CashInventory inventory;
};
