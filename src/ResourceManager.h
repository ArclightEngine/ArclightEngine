#pragma once

#include <string>
#include <vector>

namespace Arclight::ResourceManager {

void Initialize();

int LoadResource(const std::string& name, std::vector<uint8_t>& data);

} // namespace Arclight::ResourceManager