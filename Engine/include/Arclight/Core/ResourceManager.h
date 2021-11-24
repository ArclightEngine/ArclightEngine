#pragma once

#include <cassert>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <Arclight/Core/Resource.h>

namespace Arclight {

class ResourceFormat {
public:
    virtual ~ResourceFormat() = default;

    virtual bool CanLoad(const UnicodeString& path) const = 0;
    virtual std::shared_ptr<Resource> CreateResource() const = 0;
};

class ResourceManager final {
public:
    ResourceManager();
    ~ResourceManager();

    static inline ResourceManager& instance() {
        assert(s_instance);
        return *s_instance;
    }

    void RegisterFormat(ResourceFormat* format);

    const std::shared_ptr<Resource>& GetResource(const std::string& id);
    template<typename R>
    inline std::shared_ptr<R> GetResource(const std::string& id) {
        return std::move(ObjectCast<R>(GetResource(id)));
    }

private:
    static ResourceManager* s_instance;

    std::vector<ResourceFormat*> m_formats;
    std::map<std::string, std::shared_ptr<Resource>> m_resources;
    std::mutex m_lock;
};

} // namespace Arclight
