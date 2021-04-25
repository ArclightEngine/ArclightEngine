#include "ResourceManager.h"

#include <cstdio>
#include <cassert>

namespace ResourceManager {

void Initialize(){

}

int LoadResource(const std::string& name, std::vector<uint8_t>& data){
    std::string path = "Resources/" + name;

    FILE* file = fopen(path.c_str(), "r");
    if(!file){
        return 1;
    }

    fseek(file, 0, SEEK_END);
    data.resize(ftell(file));
    fseek(file, 0, SEEK_SET);

    size_t read = fread(data.data(), 1, data.size(), file);
    assert(read == data.size());

    return 0;
}

} // namespace ResourceManager