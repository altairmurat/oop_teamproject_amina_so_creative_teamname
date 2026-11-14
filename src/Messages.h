#pragma once
#include <string>

enum class Language {
    English,
    Korean
};

// all UI text is stored here in english and korean, если нужны новые ключи просто добавьте в cpp
class Messages {
public:
    static std::string get(const std::string& key, Language lang);
};
