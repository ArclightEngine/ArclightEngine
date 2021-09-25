#include <Arclight/Core/Application.h>

#include <Arclight/Graphics/Rendering/Renderer.h>

#include <SDL2/SDL.h>

#include <stdexcept>

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

    Rendering::Renderer::Instance()->Render();

    while (m_isRunning) {
        m_timer.Reset();
        m_input.Tick();

        pollEvents();

        for (auto& sys : m_globalSystems) {
            m_threadPool.Schedule(*sys);
        }

        while (!m_threadPool.Idle())
            pollEvents(); // We shouldn't really busy wait

        m_currentWorld.Cleanup();

        ProcessDeferQueue();

        if(m_pendingStateChange){
            if(m_stateManager.IsEmpty()){
                m_currentState = nullptr;
            } else {
                m_currentState = &m_states.at(m_stateManager.CurrentState());
            }

            m_pendingStateChange.s = -1;
        }

        long elapsed = m_timer.Elapsed();
        long waitTime = 1000000 / 60 - elapsed;
        if (waitTime > 0) {
            usleep(waitTime);
        }
    }
}

void Application::Exit() { m_isRunning = false; }

void Application::ProcessDeferQueue(){
    while(!m_deferQueue.empty()){
        m_deferQueue.front()();
        m_deferQueue.pop();
    }
}

void Application::PopStateImpl(){
    if(m_pendingStateChange){
        throw std::runtime_error("State change already queued!");
    }

    m_pendingStateChange = {1}; // Dummy state value to indicate presence of change
    m_deferQueue.push([this]{
        m_stateManager.Pop();
    });
}

void Application::PushStateImpl(State s){
    if(m_pendingStateChange){
        throw std::runtime_error("State change already queued!");
    }

    m_pendingStateChange = {s};
    m_deferQueue.push([this]{
        m_stateManager.Push(m_pendingStateChange.s);
    });
}

void Application::LoadStateImpl(State s){
    if(m_pendingStateChange){
        throw std::runtime_error("State change already queued!");
    }

    m_pendingStateChange = {s};

    m_deferQueue.push([this]{
        m_stateManager.Swap(m_pendingStateChange.s);
    });
}

} // namespace Arclight
