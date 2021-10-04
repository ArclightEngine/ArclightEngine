#pragma once

#include <Arclight/Components/RemovalTag.h>
#include <Arclight/Core/NonCopyable.h>
#include <Arclight/Core/Util.h>
#include <Arclight/ECS/Component.h>
#include <Arclight/ECS/Entity.h>
#include <Arclight/ECS/Registry.h>

#include <mutex>

namespace Arclight {

class World final : NonCopyable {
    friend class Application;
public:
    World() = default;

    ALWAYS_INLINE static World& Current() { return *s_currentWorld; }

    void Cleanup();

    ALWAYS_INLINE Entity CreateEntity() {
        Entity ent = m_registry.create();
        return ent;
    }

    ALWAYS_INLINE void DestroyEntity(Entity entity) {
        m_registry.emplace<Components::RemovalTag>(entity);
    }

    template <Component C, typename... Args>
    ALWAYS_INLINE void AddComponent(Entity ent, Args&&... args) {
        m_registry.emplace<C>(ent, std::move(args)...);
    }

    template <Component C>
    ALWAYS_INLINE C& GetComponent(Entity ent){
        return m_registry.get<C>(ent);
    }

    template <Component... C> ALWAYS_INLINE size_t RemoveComponents(Entity ent) {
        // Check if valid ourselves as the removal is being deferred
        assert(m_registry.valid(ent));
        // Needs to be deferred
        std::unique_lock lock(m_componentCleanupMutex);
        AssureCleanupFunction<C...>(ent);
        m_registry.emplace<Components::ComponentRemovalTag<C>...>(ent);
    }

    template <Component... C> ALWAYS_INLINE bool HasAllOf(Entity ent) {
        return m_registry.all_of<C...>(ent);
    }

    template <Component... C> ALWAYS_INLINE bool HasAnyOf(Entity ent) {
        return m_registry.any_of<C...>(ent);
    }

    template <Component... C> ALWAYS_INLINE auto View() {
        return m_registry.view<C...>();
    }

    ALWAYS_INLINE ECSRegistry& Registry() { return m_registry; }

private:
    // The current world is set by the application,
    // it is here so prevent circular header dependencies
    static World* s_currentWorld;

    // EnTT registry is not entirely thread-safe.
    // Arclight orders components into pools for cleanup.
    // At the end of each frame,
    // A Job is created per component to clean up the registry
    //
    // Afterwards, entities are destroyed consecutively

    template <Component C> ALWAYS_INLINE void AssureCleanupFunction(Entity e) {
        const auto index = entt::type_index<C>::value();

        if (!(index < m_componentCleanupFunctions.size())) {
            m_componentCleanupFunctions.resize(size_t(index) + 1u);
        }

        auto& func = m_componentCleanupFunctions[index];
        if (!func) {
            func = &RemoveComponentImpl<C>;
        }
    }

    template <Component C> void RemoveComponentImpl() {
        auto view = m_registry.view<const Components::ComponentRemovalTag<C>>();
        for (Entity entity : view) {
            m_registry.remove<Components::ComponentRemovalTag<C>, C>(entity);
        }
    }

    // Store the components that need to be removed at the end of the frame
    std::mutex m_componentCleanupMutex;
    std::vector<void (World::*)()> m_componentCleanupFunctions;

    ECSRegistry m_registry;
};

} // namespace Arclight
