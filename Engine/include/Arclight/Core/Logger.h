#pragma once

#include <Arclight/Core/UnicodeString.h>
#include <Arclight/Core/Util.h>

#include <Arclight/Platform/Platform.h>

#include <Arclight/Vector.h>

#include <cassert>

#define FMT_EXCEPTIONS 0

#include <fmt/core.h>
#include <fmt/format.h>

template <> struct fmt::formatter<Arclight::UnicodeString> : fmt::formatter<std::string> {
    template <typename Ctx> auto format(const Arclight::UnicodeString& s, Ctx& ctx) {
        std::string utf8String;
        s.toUTF8String(utf8String);
        return formatter<std::string>::format(utf8String, ctx);
    }
};

template <typename T> struct fmt::formatter<Arclight::Vector2<T>>{
    constexpr auto parse(format_parse_context& ctx) -> format_parse_context::iterator {
        assert(ctx.end() == ctx.begin());
        
        return ctx.end();
    }
    
    auto format(const Arclight::Vector2<T>& v, format_context& ctx) const -> format_context::iterator {
        return fmt::format_to(ctx.out(), "({}, {})", v.x, v.y);
    }
};

namespace Arclight{
namespace Logger {

template<typename F, typename ...Args>
ALWAYS_INLINE void Debug(const F& f, Args&&... args){
    fmt::print(stderr, "[Arclight] [Debug] ");
    fmt::vprint(stderr, f, fmt::make_args_checked<Args...>(f, args...));
    fmt::print(stderr, "\n");
}

template<typename F, typename ...Args>
ALWAYS_INLINE void Warning(const F& f, Args&&... args){
    fmt::print(stderr, "[Arclight] [Warning] ");
    fmt::vprint(stderr, f, fmt::make_args_checked<Args...>(f, args...));
    fmt::print(stderr, "\n");
}

template<typename F, typename ...Args>
ALWAYS_INLINE void Error(const F& f, Args&&... args){
    fmt::print(stderr, "[Arclight] [Error] ");
    fmt::vprint(stderr, f, fmt::make_args_checked<Args...>(f, args...));
    fmt::print(stderr, "\n");
}

} // namespace Logger
} // namespace Arclight
