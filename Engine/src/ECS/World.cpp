#include <Arclight/ECS/World.h>

namespace Arclight {

void World::Cleanup() {
    for(void(World::*func)() : m_componentCleanupFunctions){
        (this->*func)();
    }

    // Remove all tagged entities
    auto view = m_registry.view<const Components::RemovalTag>();
    for (Entity entity : view) {
        m_registry.destroy(entity);
    }
}

} // namespace Arclight
