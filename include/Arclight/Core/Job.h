#pragma once

namespace Arclight {

class Job {
    friend class ThreadPool;
public:
    virtual ~Job() = default;

    virtual void Run() = 0;

private:
};

template<typename T>
class FunctionJob final
    : public Job {
public:
    inline FunctionJob(T&& func)
        : m_func(func) {}

    void Run() override {
        m_func();
    }

private:
    T m_func;
};

} // namespace Arclight