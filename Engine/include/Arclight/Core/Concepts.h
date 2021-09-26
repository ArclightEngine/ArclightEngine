#pragma once

#include <concepts>
#include <string>

#ifdef __clang__
namespace std {

template <class F, class T>
concept convertible_to = std::is_convertible_v<F, T> &&
    requires(std::add_rvalue_reference_t<F> (&f)()) {
    static_cast<T>(f());
};

} // namespace std
#endif

namespace Arclight {

template <typename T>
concept Serializable = requires(T t) {
    { t.Serialize() } -> std::convertible_to<std::string>;
};

} // namespace Arclight
