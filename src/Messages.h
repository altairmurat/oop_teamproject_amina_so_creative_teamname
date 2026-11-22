#pragma once
#include <string>
#include "Constants.h"

// Language enum убрал отсюда тк он уже есть в Constants.h, иначе не компилится
// all UI text is stored here in english and korean, если нужны новые ключи просто добавьте в cpp
class Messages {
public:
    static std::string get(const std::string& key, Language lang);
};
