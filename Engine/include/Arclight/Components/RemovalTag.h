#pragma once

#include <Arclight/ECS/Component.h>

namespace Arclight {

namespace Components {

// Special component that indicates the entity should be deleted
struct RemovalTag {};

template <Component T> struct ComponentRemovalTag {};

} // namespace Components

} // namespace Arclight
