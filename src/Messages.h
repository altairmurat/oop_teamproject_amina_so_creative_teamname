// Owner: Alan (Person D)
#pragma once
#include <string>
#include "Constants.h"

class Messages {
public:
    static std::string get(const std::string& key, Language lang);
};
