#pragma once

#include <Arclight/ECS/Component.h>

#include <entt/entity/entity.hpp>

#include <cstdint>
#include <list>
#include <memory>

namespace Arclight {

// Entities are exposed as no more than integers
using Entity = entt::entity;

constexpr typeof(entt::null) NullEntity = entt::null;

} // namespace Arclight
