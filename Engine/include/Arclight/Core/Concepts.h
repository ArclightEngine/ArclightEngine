#pragma once

#include <concepts>
#include <string>

namespace Arclight {

template <typename T>
concept Serializable = requires(T t) {
    { t.Serialize() } -> std::convertible_to<std::string>;
};

} // namespace Arclight
