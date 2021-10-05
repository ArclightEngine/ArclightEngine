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
    enum class When {
        Init, // Enter state or enter program
        Tick, // Run each tick
        Exit, // Run on state change or exit program
    };

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

    template<void(*Function)(float, ::Arclight::World&), When when = When::Tick, State s = StateNone>
    ALWAYS_INLINE void AddSystem() {
        if constexpr(s == StateNone){
            if constexpr(when == When::Init) {
                m_globalSystems.init.push_back(new System<Function>());
            } else if constexpr(when == When::Tick) {
                m_globalSystems.tick.push_back(new System<Function>());
            } else if constexpr(when == When::Exit) {
                m_globalSystems.exit.push_back(new System<Function>());
            }
        } else {
            if constexpr(when == When::Init) {
                m_states.at(s).init.push_back(new System<Function>());
            } else if constexpr(when == When::Tick) {
                m_states.at(s).tick.push_back(new System<Function>());
            } else if constexpr(when == When::Exit) {
                m_states.at(s).exit.push_back(new System<Function>());
            }
        }
    }

    template<class Clazz, void(Clazz::*Function)(float, ::Arclight::World&), When when = When::Tick, State s = StateNone>
    ALWAYS_INLINE void AddSystem(Clazz& ref) {
        if constexpr(s == StateNone){
            if constexpr(when == When::Init) {
                m_globalSystems.init.push_back(new ClassSystem<Clazz, Function>(ref));
            } else if constexpr(when == When::Tick) {
                m_globalSystems.tick.push_back(new ClassSystem<Clazz, Function>(ref));
            } else if constexpr(when == When::Exit) {
                m_globalSystems.exit.push_back(new ClassSystem<Clazz, Function>(ref));
            }
        } else {
            if constexpr(when == When::Init) {
                m_states.at(s).init.push_back(new ClassSystem<Clazz, Function>(ref));
            } else if constexpr(when == When::Tick) {
                m_states.at(s).tick.push_back(new ClassSystem<Clazz, Function>(ref));
            } else if constexpr(when == When::Exit) {
                m_states.at(s).exit.push_back(new ClassSystem<Clazz, Function>(ref));
            }
        }
    }

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

        ALWAYS_INLINE void LoadWorld(std::shared_ptr<World> newWorld) { m_app.LoadWorldImpl(newWorld); }

    private:
        Application& m_app;
    } commands = Commands(*this);

private:
    static Application* s_instance;

    void RunStateInitSystems();
    void RunStateExitSystems();
    void ProcessJobQueue();
    void ProcessDeferQueue();

    void LoadStateImpl(State s);
    void PushStateImpl(State s);
    void PopStateImpl();
    void LoadWorldImpl(std::shared_ptr<World> world);

    // Frame delay in us
    const long m_frameDelay = 1000000 / 60;

    bool m_isRunning = true;

    Timer m_timer;

    Input m_input;
    ThreadPool m_threadPool;
    ResourceManager m_resourceManager;
    StateManager m_stateManager;

    std::shared_ptr<World> m_currentWorld;

    std::queue<std::function<void()>> m_deferQueue;

    // Enforce a policy where only one system can undergo a state change,
    // one per frame
    struct PendingStateChange {
        State s;

        operator bool() { return s >= 0; }
    } m_pendingStateChange = {-1};
    SystemGroup* m_currentState = nullptr;
    std::unordered_map<State, SystemGroup> m_states;

    SystemGroup m_globalSystems;
};

} // namespace Arclight
