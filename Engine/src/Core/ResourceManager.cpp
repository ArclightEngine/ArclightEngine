#include <Arclight/Core/ResourceManager.h>

#include <cassert>
#include <cstdio>
#include <map>

#include <nlohmann/json.hpp>

namespace Arclight {

ResourceManager* ResourceManager::s_instance = nullptr;

ResourceManager::ResourceManager() {
    assert(!s_instance);
    s_instance = this;
}

const std::unique_ptr<Resource>& ResourceManager::GetResource(const std::string& id) {
    return m_resources[id];
}

} // namespace Arclight
