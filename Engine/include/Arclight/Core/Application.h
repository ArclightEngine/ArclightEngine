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

    static ALWAYS_INLINE Application& instance() { return *s_instance; }

    // Returns WindowContext for Application
    // Should be used as opposed to WindowContext::instance();
    static WindowContext& window() { return *WindowContext::instance(); }

    void run();
    void main_loop();

    void exit();

    // For now we do not allow runtime definition of states
    // Ensure states are statically defined by using templates
    template <State s> Application& add_state() {
        static_assert(s >= 0);
        m_states[s]; // Default initialize

        return *this;
    }

    template<void(*Function)(float, ::Arclight::World&), When when = When::Tick, State s = StateNone>
    ALWAYS_INLINE Application& add_system() {
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

        return *this;
    }

    template<class Clazz, void(Clazz::*Function)(float, ::Arclight::World&), When when = When::Tick, State s = StateNone>
    ALWAYS_INLINE Application& add_system(Clazz& ref) {
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

        return *this;
    }

    // Command pattern
    // Commands are deferred until end of frame
    // they are also run consecutively,
    // as opposed to concurrently
    class Commands {
    public:
        ALWAYS_INLINE Commands(Application& app) : m_app(app) {}

        template <State s> ALWAYS_INLINE void load_state() { m_app.load_state_impl(s); }

        template <State s> ALWAYS_INLINE void push_state() { m_app.push_state_impl(s); }

        ALWAYS_INLINE void pop_state() { m_app.pop_state_impl(); }

        ALWAYS_INLINE void load_world(std::shared_ptr<World> newWorld) { m_app.load_world_impl(newWorld); }

    private:
        Application& m_app;
    } commands = Commands(*this);

private:
    static Application* s_instance;

    void run_state_init_systems();
    void run_state_exit_systems();
    void process_job_queue();
    void process_defer_queue();

    void load_state_impl(State s);
    void push_state_impl(State s);
    void pop_state_impl();
    void load_world_impl(std::shared_ptr<World> world);

    // Frame delay in us
    const long m_frameDelay = 1000000 / 120;

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
