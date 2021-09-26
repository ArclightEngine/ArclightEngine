#pragma once

#include <Arclight/Core/Object.h>
#include <Arclight/Core/Concepts.h>

#include <concepts>

namespace Arclight {

template<typename T>
concept Component = requires(T t) {
    // Components CANNOT be pointers
    // TODO: Future requirements will involve serailizable
    std::same_as<std::decay_t<T>, T>;
    T(); // Must have default constructor
#ifndef __clang__
    std::move_constructible<T>; // Must have move constructor
#else
    std::destructible<T> && std::is_constructible_v<T, T> && std::convertible_to<T, T>;
#endif
};

} // namespace Arclight
