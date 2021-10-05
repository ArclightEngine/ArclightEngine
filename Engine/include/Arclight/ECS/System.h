#pragma once

#include <list>
#include <memory>
#include <string>

#include <Arclight/Core/Job.h>
#include <Arclight/Core/NonCopyable.h>
#include <Arclight/Core/Object.h>
#include <Arclight/Core/Timer.h>
#include <Arclight/Core/Util.h>
#include <Arclight/ECS/World.h>

namespace Arclight {

struct SystemGroup {
    std::list<Job*> init;
    std::list<Job*> tick;
    std::list<Job*> exit;
};

template <void (*Function)(float, World&)>
class System final : public Job, public Object, NonCopyable {
    ARCLIGHT_OBJECT(System, Object);
public:
    void Init() final override {
        m_timer.Reset();
    }

    void Run() final override {
        m_elapsedTime = m_timer.Elapsed() / 1000000.f;

        Function(m_elapsedTime, World::Current());
    }

protected:
    ALWAYS_INLINE float Elapsed() const { return m_elapsedTime; }

    float m_elapsedTime = 0;
    Timer m_timer;
};

template <class Clazz, void (Clazz::*Function)(float, World&)>
class ClassSystem final : public Job, public Object, NonCopyable {
    ARCLIGHT_OBJECT(System, Object);

public:
    ClassSystem(Clazz& clazz) : m_data(&clazz) {}
    
    void Init() final override {
        m_timer.Reset();
    }

    void Run() final override {
        m_elapsedTime = m_timer.Elapsed() / 1000000.f;

        (m_data->*Function)(m_elapsedTime, World::Current());
    }

protected:
    ALWAYS_INLINE float Elapsed() const { return m_elapsedTime; }

    Clazz* m_data;

    float m_elapsedTime = 0;
    Timer m_timer;
};

} // namespace Arclight
