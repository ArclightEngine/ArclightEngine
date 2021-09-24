# Arclight Engine

A lightweight 2D game engine written in C++20 using Vulkan and SDL.

Features:
- Multithreaded
- Unicode support
- Vulkan rendering
- ECS architecture

Platforms:
- Linux
- Windows (WIP)

Future goals:
- Clean API
- Multi-language/Scripting support
- Heavily multithreaded
- Networking support
- Multiplatform support (Linux, Windows, WebAssembly, macOS, FreeBSD, Android?)

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

- [SDL2](http://libsdl.org/)
- [AMD's Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- [stb_image](https://github.com/nothings/stb)
- [nlohmann/json](https://github.com/nlohmann/json)
- [Freetype](https://freetype.org)
- [ICU](https://icu.unicode.org/)
- [EnTT](https://github.com/skypjack/entt)
