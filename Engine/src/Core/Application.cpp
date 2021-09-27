#include <Arclight/Core/Application.h>

#include <Arclight/Core/Logger.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Platform/Platform.h>

#include <SDL2/SDL.h>

#include <stdexcept>

#ifdef ARCLIGHT_PLATFORM_WASM
#include <emscripten.h>

static void emscripten_main_loop(){
    static Arclight::Application* appInstance = &Arclight::Application::Instance();
    appInstance->MainLoop();
}
#endif

namespace Arclight {

Application* Application::s_instance = nullptr;

Application::Application() {
    if (s_instance) {
        throw std::runtime_error("Application instance already exists!");
    }

    s_instance = this;
}

void Application::Run() {
    ProcessDeferQueue();

    Rendering::Renderer::Instance()->Render();

#ifdef ARCLIGHT_PLATFORM_WASM
    // This function makes sure we yield to the browser
    emscripten_set_main_loop(emscripten_main_loop, 1000000 / m_frameDelay, true);
#else
    while (m_isRunning) {
        m_timer.Reset();

        MainLoop();

        long elapsed = m_timer.Elapsed();
        long waitTime = m_frameDelay - elapsed;
        if (waitTime > 0) {
            usleep(waitTime);
        }
    }
#endif
}

void Application::MainLoop() {
    auto pollEvents = [&]() -> void {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                m_isRunning = false;
                break;
            case SDL_KEYDOWN:
                m_input.OnKey((KeyCode)event.key.keysym.sym, Input::KeyState_Pressed);
                break;
            case SDL_KEYUP:
                m_input.OnKey((KeyCode)event.key.keysym.sym, Input::KeyState_Released);
                break;
            default:
                break;
            }
        }
    };

    m_input.Tick();
    pollEvents();

    for (auto& sys : m_globalSystems) {
        m_threadPool.Schedule(*sys);
    }

    m_threadPool.Run();

    while (!m_threadPool.Idle())
        pollEvents(); // We shouldn't really busy wait

    m_currentWorld.Cleanup();

    ProcessDeferQueue();

    if (m_pendingStateChange) {
        if (m_stateManager.IsEmpty()) {
            m_currentState = nullptr;
        } else {
            m_currentState = &m_states.at(m_stateManager.CurrentState());
        }

        m_pendingStateChange.s = -1;
    }
}

void Application::Exit() { m_isRunning = false; }

void Application::ProcessDeferQueue() {
    while (!m_deferQueue.empty()) {
        m_deferQueue.front()();
        m_deferQueue.pop();
    }
}

void Application::PopStateImpl() {
    if (m_pendingStateChange) {
        throw std::runtime_error("State change already queued!");
    }

    m_pendingStateChange = {1}; // Dummy state value to indicate presence of change
    m_deferQueue.push([this] { m_stateManager.Pop(); });
}

void Application::PushStateImpl(State s) {
    if (m_pendingStateChange) {
        throw std::runtime_error("State change already queued!");
    }

    m_pendingStateChange = {s};
    m_deferQueue.push([this] { m_stateManager.Push(m_pendingStateChange.s); });
}

void Application::LoadStateImpl(State s) {
    if (m_pendingStateChange) {
        throw std::runtime_error("State change already queued!");
    }

    m_pendingStateChange = {s};

    m_deferQueue.push([this] { m_stateManager.Swap(m_pendingStateChange.s); });
}

} // namespace Arclight
