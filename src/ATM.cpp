// Owner: Altair (Person B)
#include "ATM.h"
#include <cctype>
#include <stdexcept>

ATM::ATM(const std::string& serialNumber,
         const std::string& primaryBankName,
         LanguageMode languageMode,
         const CashInventory& initialInventory)
    : serialNumber(serialNumber),
      primaryBankName(primaryBankName),
      languageMode(languageMode),
      currentLanguage(Language::English),
      inventory(initialInventory) {
    // make sure the serial number is strictly 6 digits
    if (serialNumber.size() != Constants::SERIAL_NUMBER_LENGTH) {
        throw std::invalid_argument("serial number must be 6 digits");
    }
    for (char ch : serialNumber) {
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            throw std::invalid_argument("serial number must be digits only");
        }
    }
    if (primaryBankName.empty()) {
        throw std::invalid_argument("primary bank name cannot be empty");
    }
}

bool ATM::isPrimaryBank(const std::string& bankName) const {
    return bankName == primaryBankName;
}

void ATM::setLanguage(Language lang) {
    // unilingual ATMs are locked to English
    if (languageMode == LanguageMode::Unilingual) {
        currentLanguage = Language::English;
        return;
    }
    currentLanguage = lang;
}
