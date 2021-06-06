#pragma once

#include <Arclight/Core/Job.h>

#include <thread>
#include <vector>

namespace Arclight {

class ThreadPool final {
    friend void ThreadMain(ThreadPool* pool);
public:
    ThreadPool();
    ~ThreadPool();

    void Schedule(Job& job);

private:

    bool m_threadsShouldDie = false;

    std::vector<std::thread> m_threads;
    std::vector<Job*> m_jobs;
};

} // namespace Arclight