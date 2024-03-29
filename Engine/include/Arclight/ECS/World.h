#pragma once

#include <Arclight/Components/RemovalTag.h>
#include <Arclight/Core/NonCopyable.h>
#include <Arclight/Core/Util.h>
#include <Arclight/ECS/Component.h>
#include <Arclight/ECS/Entity.h>
#include <Arclight/ECS/Registry.h>
#include <Arclight/Platform/API.h>

#include <mutex>

namespace Arclight {

class ARCLIGHT_API World final : NonCopyable {
    friend class Application;

public:
    World() = default;

    ALWAYS_INLINE static World& current() { return *s_currentWorld; }

    void cleanup();

    ALWAYS_INLINE Entity create_entity() {
        Entity ent = m_registry.create();
        return ent;
    }

    ALWAYS_INLINE void destroy_entity(Entity entity) {
        m_registry.emplace<Components::RemovalTag>(entity);
    }

    template <Component C, typename... Args>
    ALWAYS_INLINE decltype(auto) add_component(Entity ent, Args&&... args) {
        return m_registry.emplace<C>(ent, std::move(args)...);
    }

    template <Component C> ALWAYS_INLINE C& get_component(Entity ent) {
        return m_registry.get<C>(ent);
    }

    template <Component... C> ALWAYS_INLINE size_t remove_components(Entity ent) {
        // Check if valid ourselves as the removal is being deferred
        assert(m_registry.valid(ent));
        // Needs to be deferred
        std::unique_lock lock(m_componentCleanupMutex);
        assure_cleanup_function<C...>();
        m_registry.emplace<Components::ComponentRemovalTag<C>...>(ent);
    }

    template <Component... C> ALWAYS_INLINE bool has_all_of(Entity ent) {
        return m_registry.all_of<C...>(ent);
    }

    template <Component... C> ALWAYS_INLINE bool has_any_of(Entity ent) {
        return m_registry.any_of<C...>(ent);
    }

    template <Component... C> ALWAYS_INLINE auto view() { return m_registry.view<C...>(); }

    ALWAYS_INLINE ECSRegistry& registry() { return m_registry; }

    template <typename T, typename... Args> ALWAYS_INLINE T& ctx_set(Args&&... args) {
        return m_registry.ctx().emplace<T>(std::move(args)...);
    }

    template <typename T, typename... Args> ALWAYS_INLINE T& ctx_set_named(const entt::id_type id, Args&&... args) {
        return m_registry.ctx().emplace_hint<T>(id, std::move(args)...);
    }

    template <typename T>
    [[nodiscard]] ALWAYS_INLINE T& ctx(const entt::id_type id = entt::type_id<T>().hash()) {
        return m_registry.ctx().at<T>(id);
    }
    template <typename T>
    [[nodiscard]] ALWAYS_INLINE T* try_ctx(const entt::id_type id = entt::type_id<T>().hash()) {
        return m_registry.ctx().find<T>(id);
    }

    template <typename T>
    ALWAYS_INLINE void ctx_unset(const entt::id_type id = entt::type_id<T>().hash()) {
        m_registry.ctx().erase<T>(id);
    }

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

    template <Component C> ALWAYS_INLINE void assure_cleanup_function() {
        const auto index = entt::type_index<C>::value();

        if (!(index < m_componentCleanupFunctions.size())) {
            m_componentCleanupFunctions.resize(size_t(index) + 1u);
        }

        auto& func = m_componentCleanupFunctions[index];
        if (!func) {
            func = &remove_component_impl<C>;
        }
    }

    template <Component C> void remove_component_impl() {
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
