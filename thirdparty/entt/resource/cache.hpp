#ifndef ENTT_RESOURCE_CACHE_HPP
#define ENTT_RESOURCE_CACHE_HPP

#include <type_traits>
#include <utility>
#include "../config/config.h"
#include "../container/dense_map.hpp"
#include "../core/fwd.hpp"
#include "../core/utility.hpp"
#include "fwd.hpp"
#include "handle.hpp"
#include "loader.hpp"

namespace entt {

/**
 * @brief Simple cache for resources of a given type.
 *
 * Minimal implementation of a cache for resources of a given type. It doesn't
 * offer much functionalities but it's suitable for small or medium sized
 * applications and can be freely inherited to add targeted functionalities for
 * large sized applications.
 *
 * @tparam Resource Type of resources managed by a cache.
 */
template<typename Resource>
class resource_cache {
    static_assert(std::is_same_v<Resource, std::remove_const_t<std::remove_reference_t<Resource>>>, "Invalid resource type");

public:
    /*! @brief Unsigned integer type. */
    using size_type = std::size_t;
    /*! @brief Type of resources managed by a cache. */
    using resource_type = Resource;

    /*! @brief Default constructor. */
    resource_cache() = default;

    /*! @brief Default move constructor. */
    resource_cache(resource_cache &&) = default;

    /*! @brief Default move assignment operator. @return This cache. */
    resource_cache &operator=(resource_cache &&) = default;

    /**
     * @brief Number of resources managed by a cache.
     * @return Number of resources currently stored.
     */
    [[nodiscard]] size_type size() const ENTT_NOEXCEPT {
        return resources.size();
    }

    /**
     * @brief Returns true if a cache contains no resources, false otherwise.
     * @return True if the cache contains no resources, false otherwise.
     */
    [[nodiscard]] bool empty() const ENTT_NOEXCEPT {
        return resources.empty();
    }

    /**
     * @brief Clears a cache and discards all its resources.
     *
     * Handles are not invalidated and the memory used by a resource isn't
     * freed as long as at least a handle keeps the resource itself alive.
     */
    void clear() ENTT_NOEXCEPT {
        resources.clear();
    }

    /**
     * @brief Loads the resource that corresponds to a given identifier.
     *
     * In case an identifier isn't already present in the cache, it loads its
     * resource and stores it aside for future uses. Arguments are forwarded
     * directly to the loader in order to construct properly the requested
     * resource.
     *
     * @note
     * If the identifier is already present in the cache, this function does
     * nothing and the arguments are simply discarded.
     *
     * @warning
     * If the resource cannot be loaded correctly, the returned handle will be
     * invalid and any use of it will result in undefined behavior.
     *
     * @tparam Loader Type of loader to use to load the resource if required.
     * @tparam Args Types of arguments to use to load the resource if required.
     * @param id Unique resource identifier.
     * @param args Arguments to use to load the resource if required.
     * @return A handle for the given resource.
     */
    template<typename Loader, typename... Args>
    resource_handle<resource_type> load(const id_type id, Args &&...args) {
        if(auto it = resources.find(id); it == resources.cend()) {
            if(auto handle = temp<Loader>(std::forward<Args>(args)...); handle) {
                return (resources[id] = std::move(handle));
            }
        } else {
            return it->second;
        }

        return {};
    }

    /**
     * @brief Reloads a resource or loads it for the first time if not present.
     *
     * Equivalent to the following snippet (pseudocode):
     *
     * @code{.cpp}
     * cache.discard(id);
     * cache.load(id, args...);
     * @endcode
     *
     * Arguments are forwarded directly to the loader in order to construct
     * properly the requested resource.
     *
     * @warning
     * If the resource cannot be loaded correctly, the returned handle will be
     * invalid and any use of it will result in undefined behavior.
     *
     * @tparam Loader Type of loader to use to load the resource.
     * @tparam Args Types of arguments to use to load the resource.
     * @param id Unique resource identifier.
     * @param args Arguments to use to load the resource.
     * @return A handle for the given resource.
     */
    template<typename Loader, typename... Args>
    resource_handle<resource_type> reload(const id_type id, Args &&...args) {
        return (discard(id), load<Loader>(id, std::forward<Args>(args)...));
    }

    /**
     * @brief Creates a temporary handle for a resource.
     *
     * Arguments are forwarded directly to the loader in order to construct
     * properly the requested resource. The handle isn't stored aside and the
     * cache isn't in charge of the lifetime of the resource itself.
     *
     * @tparam Loader Type of loader to use to load the resource.
     * @tparam Args Types of arguments to use to load the resource.
     * @param args Arguments to use to load the resource.
     * @return A handle for the given resource.
     */
    template<typename Loader, typename... Args>
    [[nodiscard]] resource_handle<resource_type> temp(Args &&...args) const {
        return Loader{}.get(std::forward<Args>(args)...);
    }

    /**
     * @brief Creates a handle for a given resource identifier.
     *
     * A resource handle can be in a either valid or invalid state. In other
     * terms, a resource handle is properly initialized with a resource if the
     * cache contains the resource itself. Otherwise the returned handle is
     * uninitialized and accessing it results in undefined behavior.
     *
     * @sa resource_handle
     *
     * @param id Unique resource identifier.
     * @return A handle for the given resource.
     */
    [[nodiscard]] resource_handle<const resource_type> handle(const id_type id) const {
        if(auto it = resources.find(id); it != resources.cend()) {
            return it->second;
        }

        return {};
    }

    /*! @copydoc handle */
    [[nodiscard]] resource_handle<resource_type> handle(const id_type id) {
        if(auto it = resources.find(id); it != resources.end()) {
            return it->second;
        }

        return {};
    }

    /**
     * @brief Checks if a cache contains a given identifier.
     * @param id Unique resource identifier.
     * @return True if the cache contains the resource, false otherwise.
     */
    [[nodiscard]] bool contains(const id_type id) const {
        return (resources.find(id) != resources.cend());
    }

    /**
     * @brief Discards the resource that corresponds to a given identifier.
     *
     * Handles are not invalidated and the memory used by the resource isn't
     * freed as long as at least a handle keeps the resource itself alive.
     *
     * @param id Unique resource identifier.
     */
    void discard(const id_type id) {
        if(auto it = resources.find(id); it != resources.end()) {
            resources.erase(it);
        }
    }

    /**
     * @brief Iterates all resources.
     *
     * The function object is invoked for each element. It is provided with
     * either the resource identifier, the resource handle or both of them.<br/>
     * The signature of the function must be equivalent to one of the following
     * forms:
     *
     * @code{.cpp}
     * void(const entt::id_type);
     * void(entt::resource_handle<const resource_type>);
     * void(const entt::id_type, entt::resource_handle<const resource_type>);
     * @endcode
     *
     * @tparam Func Type of the function object to invoke.
     * @param func A valid function object.
     */
    template<typename Func>
    void each(Func func) const {
        auto begin = resources.begin();
        auto end = resources.end();

        while(begin != end) {
            auto curr = begin++;

            if constexpr(std::is_invocable_v<Func, id_type>) {
                func(curr->first);
            } else if constexpr(std::is_invocable_v<Func, resource_handle<const resource_type>>) {
                func(resource_handle<const resource_type>{curr->second});
            } else {
                func(curr->first, resource_handle<const resource_type>{curr->second});
            }
        }
    }

    /**
     * @copybrief each
     *
     * The function object is invoked for each element. It is provided with
     * either the resource identifier, the resource handle or both of them.<br/>
     * The signature of the function must be equivalent to one of the following
     * forms:
     *
     * @code{.cpp}
     * void(const entt::id_type);
     * void(entt::resource_handle<resource_type>);
     * void(const entt::id_type, entt::resource_handle<resource_type>);
     * @endcode
     *
     * @tparam Func Type of the function object to invoke.
     * @param func A valid function object.
     */
    template<typename Func>
    void each(Func func) {
        auto begin = resources.begin();
        auto end = resources.end();

        while(begin != end) {
            auto curr = begin++;

            if constexpr(std::is_invocable_v<Func, id_type>) {
                func(curr->first);
            } else if constexpr(std::is_invocable_v<Func, resource_handle<resource_type>>) {
                func(curr->second);
            } else {
                func(curr->first, curr->second);
            }
        }
    }

private:
    dense_map<id_type, resource_handle<resource_type>, identity> resources;
};

} // namespace entt

#endif
