#pragma once

#include <Arclight/Core/UnicodeString.h>

#include <unordered_map>

namespace Arclight {

class Locale {
    Locale();

    void LoadFromJSON();

private:
    static Locale* s_current;

    std::unordered_map<std::string, UnicodeString> m_text;
};

} // namespace Arclight