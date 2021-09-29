#pragma once

#include <Arclight/Core/Input.h>
#include <Arclight/Core/NonCopyable.h>
#include <Arclight/Core/ResourceManager.h>
#include <Arclight/Core/ThreadPool.h>
#include <Arclight/Core/Timer.h>
#include <Arclight/Core/Util.h>
#include <Arclight/ECS/Registry.h>
#include <Arclight/ECS/System.h>
#include <Arclight/ECS/World.h>
#include <Arclight/State/StateManager.h>
#include <Arclight/Window/WindowContext.h>

#include <functional>
#include <map>
#include <memory>
#include <mutex>

namespace Arclight {

class Application : NonCopyable {
public:
    Application();

    static ALWAYS_INLINE Application& Instance() { return *s_instance; }

    // Returns WindowContext for Application
    // Should be used as opposed to WindowContext::Instance();
    static WindowContext& Window() { return *WindowContext::Instance(); }

    void Run();
    void MainLoop();

    void Exit();

    // For now we do not allow runtime definition of states
    // Ensure states are statically defined by using templates
    template <State s> void AddState() {
        static_assert(s >= 0);
        m_states[s]; // Default initialize
    }

    template<State s, void(*Function)(float, ::Arclight::World&)>
    ALWAYS_INLINE void AddSystem() { m_states.at(s).systems.push_back(new System<Function>()); }

    template<State s, class Clazz, void(Clazz::*Function)(float, ::Arclight::World&)>
    ALWAYS_INLINE void AddSystem(Clazz& ref) { m_states.at(s).systems.push_back(new ClassSystem<Clazz, Function>(ref)); }

    template<void(*Function)(float, ::Arclight::World&)>
    ALWAYS_INLINE void AddSystem() { m_globalSystems.push_back(new System<Function>()); }

    template<class Clazz, void(Clazz::*Function)(float, ::Arclight::World&)>
    ALWAYS_INLINE void AddSystem(Clazz& ref) { m_globalSystems.push_back(new ClassSystem<Clazz, Function>(ref)); }

    // Command pattern
    // Commands are deferred until end of frame
    // they are also run consecutively,
    // as opposed to concurrently
    class Commands {
    public:
        ALWAYS_INLINE Commands(Application& app) : m_app(app) {}

        template <State s> ALWAYS_INLINE void LoadState() { m_app.LoadStateImpl(s); }

        template <State s> ALWAYS_INLINE void PushState() { m_app.PushStateImpl(s); }

        ALWAYS_INLINE void PopState() { m_app.PopStateImpl(); }

    private:
        Application& m_app;
    } commands = Commands(*this);

private:
    static Application* s_instance;

    void ProcessDeferQueue();

    void LoadStateImpl(State s);
    void PushStateImpl(State s);
    void PopStateImpl();

    struct StateData {
        std::list<Job*> systems;
    };

    // Frame delay in us
    const long m_frameDelay = 1000000 / 60;

    bool m_isRunning = true;

    Timer m_timer;

    Input m_input;
    ThreadPool m_threadPool;
    ResourceManager m_resourceManager;
    StateManager m_stateManager;

    std::queue<std::function<void()>> m_deferQueue;

    // Enforce a policy where only one system can undergo a state change,
    // one per frame
    struct PendingStateChange {
        State s;

        operator bool() { return s >= 0; }
    } m_pendingStateChange = {-1};
    StateData* m_currentState = nullptr;
    std::unordered_map<State, StateData> m_states;

    std::list<Job*> m_globalSystems;
};

} // namespace Arclight
