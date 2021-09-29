#pragma once

namespace Arclight {

class Job {
    friend class ThreadPool;
public:
    virtual ~Job() = default;

    virtual void Run() = 0;

protected:
    Job() = default;
};

} // namespace Arclight
