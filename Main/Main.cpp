

#include <assert.h>

#include <Arclight/Core/Application.h>
#include <Arclight/Core/Input.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Core/ResourceManager.h>
#include <Arclight/Core/ThreadPool.h>
#include <Arclight/Core/Timer.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Platform/Platform.h>
#include <Arclight/State/StateManager.h>
#include <Arclight/Window/WindowContext.h>

#ifdef ARCLIGHT_PLATFORM_UNIX
#include <dlfcn.h>
#include <unistd.h>
#endif

#ifdef ARCLIGHT_PLATFORM_WINDOWS
#include <windows.h>
#endif

#include <chrono>
#include <vector>

using namespace Arclight;

bool isRunning = true;

#ifdef ARCLIGHT_SINGLE_EXECUTABLE
extern "C" void game_init();
#endif

#if defined(ARCLIGHT_PLATFORM_WINDOWS)
int wmain(int argc, wchar_t** argv) {
#else
int main(int argc, char** argv) {
#endif

    Platform::Initialize();
    Logger::Debug("Using renderer: {}", Rendering::Renderer::instance()->get_name());

    Application app;

#if defined(ARCLIGHT_PLATFORM_WASM)
    void (*InitFunc)(void) = game_init;
#elif defined(ARCLIGHT_PLATFORM_UNIX)
    if (argc >= 2) {
        chdir(argv[1]);
    }

    char cwd[4096];
    getcwd(cwd, 4096);

    std::string gamePath = std::string(cwd) + "/" + "game.so";
    Logger::Debug("Loading game executable: {}", gamePath);

    void* game = dlopen(gamePath.c_str(), RTLD_GLOBAL | RTLD_NOW);
    if (!game) {
        // Try Build/game.so instead
        gamePath = std::string(cwd) + "/Build/" + "game.so";
        game = dlopen(gamePath.c_str(), RTLD_GLOBAL | RTLD_NOW);

        if (!game) {
            Logger::Debug("Error loading {}", dlerror());
            return 1;
        }
    }

    void (*InitFunc)(void) = (void (*)())dlsym(game, "game_init");
#elif defined(ARCLIGHT_PLATFORM_WINDOWS)

#ifndef ARCLIGHT_SINGLE_EXECUTABLE
    if (argc >= 2) {
        SetCurrentDirectoryW(argv[1]);
    }

    wchar_t cwd[_MAX_PATH];
    DWORD cwdLen; 
    if (cwdLen = GetCurrentDirectoryW(_MAX_PATH, cwd); cwdLen > _MAX_PATH || cwdLen == 0) {
        Logger::Error("Failed to get current working directory!");
        return 1;
    }

    Arclight::UnicodeString dllPath = cwd;
    dllPath += L"\\game.dll";

    HINSTANCE game = LoadLibraryW(as_wide_string(dllPath));
    if (!game) {
        Logger::Debug("Error loading {}", dllPath);
        return 2;
    }

    void (*InitFunc)(void) = (void (*)())GetProcAddress(game, "game_init");

    if (!InitFunc) {
        Logger::Debug("Could not resolve symbol GameInit from {}", dllPath);
        return 2;
    }
#else
    void (*InitFunc)(void) = game_init;
#endif

#else
    #error "Unsupported platform!"
#endif

    assert(InitFunc);

    InitFunc();

#if defined(ARCLIGHT_PLATFORM_WASM)
    return 0;
#else
    Platform::Cleanup();


    return 0;
#endif
}
