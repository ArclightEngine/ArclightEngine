# Arclight Engine

A lightweight 2D game engine written in C++ using Vulkan and SDL.

Features:
- Multithreaded
- Unicode support
- Vulkan rendering

Platforms:
- Linux
- Windows (WIP)

Future goals:
- Multi-language support
- Heavily multithreaded
- Networking support
- Multiplatform support (Windows, Linux, macOS, FreeBSD, Android?, Web?)

## Building
Requirements:
- Meson
- Ninja
- Vulkan SDK
- SDL2
- ICU
- Freetype

### Linux
```shell
CC=clang CXX=clang++ meson build
ninja -C build
```

## Third Party

Arclight engine uses the following third party libraries:

- [AMD's Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- [stb_image](https://github.com/nothings/stb)
- [nlohmann/json](https://github.com/nlohmann/json)
- [Freetype](https://freetype.org)
- [ICU](https://icu.unicode.org/)
