#pragma once

#include <Arclight/Core/Object.h>

#include <concepts>

namespace Arclight {

template<typename T>
concept Component = requires(T t) {
    // Components CANNOT be pointers
    // TODO: Future requirements will involve serailizable
    std::same_as<std::decay_t<T>, T>;
    T(); // Must have default constructor
    std::move_constructible<T>; // Must have move constructor
};

} // namespace Arclight
