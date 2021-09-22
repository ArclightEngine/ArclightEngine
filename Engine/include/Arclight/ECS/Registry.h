#pragma once

#include <Arclight/ECS/Component.h>
#include <Arclight/ECS/Entity.h>
#include <Arclight/ECS/System.h>

#include <entt/entity/registry.hpp>

namespace Arclight {

class ECSRegistry : public entt::basic_registry<Entity> {

};

} // namespace Arclight
