#pragma once

#ifdef __unix__
#define ARCLIGHT_PLATFORM_UNIX
#endif

#ifdef __linux__
#define ARCLIGHT_PLATFORM_LINUX
#endif

namespace Arclight::Platform {

void Initialize();
void Cleanup();

} // namespace Arclight::Platform
