#pragma once

namespace Arclight {

class Job {
    friend class ThreadPool;
public:
    virtual ~Job() = default;

    // May be called to reset any timers, etc.
    virtual void Init() {};
    virtual void run() = 0;

protected:
    Job() = default;
};

} // namespace Arclight
