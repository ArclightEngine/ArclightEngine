# Arclight Engine

A lightweight 2D game engine written in C++ using Vulkan and SDL.

Future goals:
- Multi-language support
- Multithreading
- Multiplatform support (Windows, Linux, macOS, FreeBSD, Lemon OS, Android)

## Building
Requirements:
- Meson
- Ninja
- Vulkan SDK
- SDL2

### Linux
```shell
CC=clang CXX=clang++ meson build
ninja -C build
```

## Third Party

Arclight engine uses [AMD's Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
and [stb_image](https://github.com/nothings/stb)