#pragma once

#include <Arclight/Core/Util.h>
#include <Arclight/ECS/Component.h>
#include <Arclight/ECS/Entity.h>

#define ENTT_USE_ATOMIC
#include <entt/entity/registry.hpp>

namespace Arclight {

class ECSRegistry : public entt::basic_registry<Entity> {};

} // namespace Arclight
