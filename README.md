# Arclight Engine

A lightweight 2D game engine written in C++20 using Vulkan and SDL.

Features:
- Multithreaded
- Unicode support
- Supports Vulkan and WebGL
- ECS architecture

Platforms:
- Linux
- WebAssembly/Emscripten
- Windows

Future goals:
- Clean API
- Multi-language/Scripting support
- Heavily multithreaded
- Networking support
- Multiplatform support (Windows, macOS, FreeBSD, Android?)

## Building The Engine
Requirements:
- CMake
- Ninja (optional, highly recommended)

Native build:
- SDL2
- ICU
- Freetype
- Vulkan SDK

WebAssembly build:
- Emscripten

### Linux
**Host Build**
```shell
cmake -B Build -G Ninja -DUSE_VULKAN=ON
ninja -C Build
```

**WASM/Emscripten Build**
```shell
cmake -B Build/wasm -G Ninja -DUSE_OPENGL=ON
ninja -C Build/wasm
```

## Building The Examples
**Host Build**
```
./arclight-build.py --dir Examples/Ball build
./arclight-build.py --dir Examples/Ball run
```
**WASM/Emscripten Build**
```
./arclight-build.py --platform wasm --dir Examples/Ball build
./arclight-build.py --platform wasm --dir Examples/Ball run
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
- [libfmt](https://fmt.dev)
