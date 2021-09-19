#pragma once

#include <string>
#include <map>
#include <memory>
#include <cassert>

#include <Arclight/Core/Resource.h>

namespace Arclight {

class ResourceManager final {
public:
    ResourceManager();

    inline ResourceManager& Instance() {
        assert(s_instance);
        return *s_instance;
    }

    void LoadManifest(const std::string& filesystemPath);
    const std::unique_ptr<Resource>& GetResource(const std::string& id);

private:
    static ResourceManager* s_instance;

    std::map<std::string, std::unique_ptr<Resource>> m_resources;
};

void Initialize();

std::shared_ptr<Resource> GetResource(const std::string& name);

} // namespace Arclight
