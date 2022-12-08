#include <Arclight/Core/Application.h>

#include <Arclight/Core/Fatal.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Core/Time.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Platform/Platform.h>

#include <SDL.h>


//#define ARCLIGHT_STATE_DEBUG

#ifdef ARCLIGHT_PLATFORM_WASM
#include <emscripten.h>

static void emscripten_main_loop() {
    static Arclight::Application* appInstance = &Arclight::Application::instance();
    appInstance->main_loop();
}
#endif

namespace Arclight {

Application* Application::s_instance = nullptr;

Application::Application() {
    if (s_instance) {
        FatalRuntimeError("Application instance already exists!");
    }

    s_instance = this;
    m_currentWorld = std::shared_ptr<World>(new World());
    World::s_currentWorld = m_currentWorld.get();
}

void Application::run() {
    for (auto& sys : m_globalSystems.init) {
        m_threadPool.Schedule(*sys);
    }

    // A system may have queued a world or state change
    process_defer_queue();

    Rendering::Renderer::instance()->render();

#ifdef ARCLIGHT_PLATFORM_WASM
    // This function makes sure we yield to the browser
    emscripten_set_main_loop(emscripten_main_loop, 1000000 / m_frameDelay, true);
#else
    while (m_isRunning) {
        m_timer.Reset();

        main_loop();

        long elapsed = m_timer.elapsed();
        long waitTime = m_frameDelay - elapsed;
        if (waitTime > 0) {
            sleep_for_useconds(waitTime);
        }
    }
#endif
}

void Application::main_loop() {
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
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    Rendering::Renderer::instance()->resize_viewport(
                        {event.window.data1, event.window.data2});
                }
                break;
            default:
                break;
            }
        }
    };

    m_input.Tick();
    pollEvents();

    queue_system_group<Stage::PreTick>(m_globalSystems);
    if (m_currentState) {
        queue_system_group<Stage::PreTick>(*m_currentState);
    }

    Rendering::Renderer::instance()->render();

    process_job_queue();
    World::s_currentWorld->cleanup();

    queue_system_group<Stage::Tick>(m_globalSystems);
    if (m_currentState) {
        queue_system_group<Stage::Tick>(*m_currentState);
    }

    process_job_queue();
    World::s_currentWorld->cleanup();

    queue_system_group<Stage::PostTick>(m_globalSystems);
    if (m_currentState) {
        queue_system_group<Stage::PostTick>(*m_currentState);
    }

    process_job_queue();
    World::s_currentWorld->cleanup();
    process_defer_queue();

    while (m_pendingStateChange) {
#ifdef ARCLIGHT_STATE_DEBUG
        Logger::Debug("Running exit systems!");
#endif
        run_state_exit_systems();

#ifdef ARCLIGHT_STATE_DEBUG
        Logger::Debug("Processing state change!");
#endif
        if (m_stateManager.IsEmpty()) {
            m_currentState = nullptr;
        } else {
            m_currentState = &m_states.at(m_stateManager.CurrentState());
        }

        m_pendingStateChange.s = -1;

        // A system may have queued a world or state change
        run_state_init_systems();
    }
}

void Application::exit() { m_isRunning = false; }

void Application::run_state_init_systems() {
    if (m_currentState) {
        for (auto& sys : m_currentState->init) {
            sys->Init();
            m_threadPool.Schedule(*sys);
        }

        // Ensure any timers get reset
        init_system_group(*m_currentState);

        process_job_queue();
        World::s_currentWorld->cleanup();
        process_defer_queue();
    }
}

void Application::run_state_exit_systems() {
    if (m_currentState) {
        for (auto& sys : m_currentState->exit) {
            m_threadPool.Schedule(*sys);
        }

        process_job_queue();
        World::s_currentWorld->cleanup();
        process_defer_queue();
    }
}

void Application::process_job_queue() {
    m_threadPool.run();

    while (m_threadPool.job_count())
        ; // We shouldn't really busy wait
}

void Application::process_defer_queue() {
    while (!m_deferQueue.empty()) {
        m_deferQueue.front()();
        m_deferQueue.pop();
    }
}

void Application::pop_state_impl() {
    if (m_pendingStateChange) {
        FatalRuntimeError("State change already queued!");
    }

    m_pendingStateChange = {1}; // Dummy state value to indicate presence of change
    m_deferQueue.push([this] { m_stateManager.Pop(); });
}

void Application::push_state_impl(State s) {
    if (m_pendingStateChange) {
        FatalRuntimeError("State change already queued!");
    }

    m_pendingStateChange = {s};
    m_deferQueue.push([this] { m_stateManager.Push(m_pendingStateChange.s); });
}

void Application::load_state_impl(State s) {
    if (m_pendingStateChange) {
        FatalRuntimeError("State change already queued!");
    }

    m_pendingStateChange = {s};

    m_deferQueue.push([this] { m_stateManager.Swap(m_pendingStateChange.s); });
}

void Application::load_world_impl(std::shared_ptr<World> world) {
    m_deferQueue.push([this, world] {
        m_currentWorld = std::move(world);
        World::s_currentWorld = m_currentWorld.get();
    });
}

} // namespace Arclight
