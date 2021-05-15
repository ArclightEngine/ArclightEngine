#pragma once

#include <string>
#include <vector>

#include <Arclight/Resource.h>

namespace Arclight::ResourceManager {

void Initialize();

int LoadResource(const std::string& name, Resource*& resource);

} // namespace Arclight::ResourceManager