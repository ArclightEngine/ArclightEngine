

#include <assert.h>
#include <dlfcn.h>
#include <unistd.h>

#include <Arclight/Core/ThreadPool.h>

#include <Arclight/Core/Input.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Core/ResourceManager.h>
#include <Arclight/Core/Timer.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Platform/Platform.h>
#include <Arclight/Window/WindowContext.h>

#include <chrono>
#include <vector>

using namespace Arclight;

bool isRunning = true;

int main(int argc, char** argv) {
    if (argc >= 2) {
        chdir(argv[1]);
    }

    char cwd[4096];
    getcwd(cwd, 4096);

    std::string gamePath = std::string(cwd) + "/" + "game.so";
    Logger::Debug("Loading game executable: ", gamePath);

    void* game = dlopen(gamePath.c_str(), RTLD_GLOBAL | RTLD_NOW);
    if (!game) {
        // Try Build/game.so instead
        gamePath = std::string(cwd) + "/Build/" + "game.so";
        game = dlopen(gamePath.c_str(), RTLD_GLOBAL | RTLD_NOW);
    }

    if (!game) {
        Logger::Debug("Error loading ", dlerror());
        return 1;
    }

    Platform::Initialize();

    ThreadPool threadPool;
    Input inputManager;
    ResourceManager resourceManager;

    auto pollEvents = [&]() -> void {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_KEYDOWN:
                inputManager.OnKey((KeyCode)event.key.keysym.sym, Input::KeyState_Pressed);
                break;
            case SDL_KEYUP:
                inputManager.OnKey((KeyCode)event.key.keysym.sym, Input::KeyState_Released);
                break;
            default:
                break;
            }
        }
    };

    void (*GameInit)(void) = (void (*)())dlsym(game, "GameInit");
    void (*GameRun)(void) = (void (*)())dlsym(game, "GameRun");

    assert(GameInit && GameRun);

    GameInit();

    Timer timer;
    while (isRunning) {
        timer.Reset();
        inputManager.Tick();

        pollEvents();

        GameRun();
        Rendering::Renderer::Instance()->Render();

        while (!threadPool.Idle())
            pollEvents(); // We shouldn't really busy wait

        long elapsed = timer.Elapsed();
        long waitTime = 1000000 / 60 - elapsed;
        if (waitTime > 0) {
            usleep(waitTime);
        }
    }

    return 0;
}
