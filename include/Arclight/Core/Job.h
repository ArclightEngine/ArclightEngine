#pragma once

namespace Arclight {

class Job {
public:
    virtual ~Job() = default;

    virtual void Run() = 0;
};

} // namespace Arclight