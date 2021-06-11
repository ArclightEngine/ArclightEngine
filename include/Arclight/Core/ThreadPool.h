#pragma once

#include <Arclight/Core/Job.h>

#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <condition_variable>

namespace Arclight {

class ThreadPool final {
    friend void ThreadMain(ThreadPool* pool);
public:
    ThreadPool();
    ~ThreadPool();

    void Schedule(Job& job);

    inline static ThreadPool* Instance() { return m_instance; }

private:
    static ThreadPool* m_instance;

    bool m_threadsShouldDie = false;

    std::vector<std::thread> m_threads; // Thread objects

    std::queue<Job*> m_jobs; // FIFO Job Queue
    std::condition_variable m_condition; // Used to wait for available jobs
    std::mutex m_queueMutex; // Queue Mutex
};

} // namespace Arclight