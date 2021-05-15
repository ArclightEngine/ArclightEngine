#include <Arclight/ResourceManager.h>

#include <cstdio>
#include <cassert>

#include <map>

namespace Arclight::ResourceManager {

std::map<std::string, Resource*> cache;

void Initialize(){

}

int LoadResource(const std::string& name, Resource*& resource){
    std::string path = "Resources/" + name;

    FILE* file = fopen(path.c_str(), "r");
    if(!file){
        return 1;
    }

    resource = new Resource();

    fseek(file, 0, SEEK_END);
    resource->m_data.resize(ftell(file));
    fseek(file, 0, SEEK_SET);

    size_t read = fread(resource->m_data.data(), 1, resource->m_data.size(), file);
    assert(read == resource->m_data.size());

    cache[name] = resource;

    return 0;
}

} // namespace Arclight::ResourceManager