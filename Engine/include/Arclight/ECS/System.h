#pragma once

#include <list>
#include <memory>
#include <string>

#include <Arclight/Core/Object.h>
#include <Arclight/Core/Job.h>
#include <Arclight/Core/NonCopyable.h>
#include <Arclight/Core/Timer.h>

namespace Arclight {

class System : public Job, public Object, NonCopyable {
    ARCLIGHT_OBJECT(System, Object);
public:
    void Run() final override;

    virtual void Start();
    virtual void Tick();

    inline const std::string& Name() const { return m_name; }

protected:
    inline float Elapsed() const { return m_elapsedTime; }

private:
    System(std::string name);

    float m_elapsedTime;
    Timer m_timer;

    std::string m_name; 
};

} // namespace Arclight
