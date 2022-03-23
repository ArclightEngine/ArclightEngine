#pragma once

#include <Arclight/Core/Util.h>
#include <Arclight/Platform/Platform.h>

#ifdef ARCLIGHT_PLATFORM_UNIX
#define U_CHARSET_IS_UTF8 1
#endif

#ifndef NO_ICU

#include <unicode/unistr.h>

namespace Arclight {

using UnicodeString = icu::UnicodeString;

#ifdef ARCLIGHT_PLATFORM_WINDOWS
// Windows uses UTF-16
ALWAYS_INLINE static const wchar_t* as_wide_string(UnicodeString& s) { return (const wchar_t*)s.getTerminatedBuffer(); }
#endif

} // namespace Arclight

#else

#include <string>

namespace Arclight {

class UnicodeString : public std::string {
public:
    UnicodeString() = default;
    UnicodeString(const std::string& s)
        : std::string(s) {}
    UnicodeString(std::string&& s)
        : std::string(s) {}
    UnicodeString(const char* s)
        : std::string(s) {}

    template<typename string_type>
    string_type& toUTF8String(string_type& str) const {
        str = *this;
        return str;
    }

    bool endsWith(const UnicodeString& str) const {
        return compare(length() - str.length(), str.length(), str) == 0;
    }
};

}

#endif