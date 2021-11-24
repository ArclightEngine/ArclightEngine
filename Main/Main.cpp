

#include <assert.h>
#include <dlfcn.h>
#include <unistd.h>

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

#include <chrono>
#include <vector>

using namespace Arclight;

bool isRunning = true;

extern "C" void GameInit();

int main(int argc, char** argv) {
    if (argc >= 2) {
        chdir(argv[1]);
    }

    char cwd[4096];
    getcwd(cwd, 4096);

    Platform::Initialize();
    Logger::Debug("Using renderer: ", Rendering::Renderer::instance()->GetName());

    Application app;

#if defined(ARCLIGHT_PLATFORM_WASM)
    void (*InitFunc)(void) = GameInit;
#elif defined(ARCLIGHT_PLATFORM_UNIX)
    std::string gamePath = std::string(cwd) + "/" + "game.so";
    Logger::Debug("Loading game executable: ", gamePath);

    void* game = dlopen(gamePath.c_str(), RTLD_GLOBAL | RTLD_NOW);
    if (!game) {
        // Try Build/game.so instead
        gamePath = std::string(cwd) + "/Build/" + "game.so";
        game = dlopen(gamePath.c_str(), RTLD_GLOBAL | RTLD_NOW);

        if (!game) {
            Logger::Debug("Error loading ", dlerror());
            return 1;
        }
    }

    void (*InitFunc)(void) = (void (*)())dlsym(game, "GameInit");
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
