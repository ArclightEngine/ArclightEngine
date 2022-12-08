#include <Arclight/Core/ThreadPool.h>

#include <Arclight/Core/Fatal.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Platform/Platform.h>

#include <cstdlib>

namespace Arclight {

ThreadPool* ThreadPool::m_instance = nullptr;

void ThreadMain(ThreadPool* pool) {
    Arclight::Job* currentJob = nullptr;
    while (!pool->m_threadsShouldDie) {
        {
            std::unique_lock<std::mutex> acquiredLock(pool->m_queueMutex);
            pool->m_condition.wait(acquiredLock, [pool]() -> bool {
                return !pool->m_jobs.empty() || pool->m_threadsShouldDie;
            }); // Wait for jobs

            if (pool->m_jobs.empty()) {
                continue;
            }

            currentJob = pool->m_jobs.front();
            pool->m_jobs.pop();
        }

        // Ensure we release lock before job is run
        currentJob->run();
        pool->m_jobCount--;
    }
}

ThreadPool::ThreadPool() {
    if (m_instance) {
        FatalRuntimeError("Instance of ThreadPool already exists!");
        return;
    }

    long cpuCount = 0;

    if(Platform::multithreading_enabled()){
        // Hint of hardware threads
        cpuCount = static_cast<long>(std::thread::hardware_concurrency()) - 1; // Prevent overflow
        if (cpuCount < 1) { // Failed to query hardware thread count
            cpuCount = 0;
        }
    }

    Logger::Debug("[ThreadPool] Using {} threads.", cpuCount);

    while (cpuCount--) {
        m_threads.push_back(std::thread(ThreadMain, this));
    }

    m_instance = this;
}

ThreadPool::~ThreadPool() {
    if (m_instance == this) {
        m_instance = nullptr;
    }

    m_threadsShouldDie = true;

    m_condition.notify_all();
    for (auto& thread : m_threads) {
        thread.join();
    }
}

void ThreadPool::Schedule(Job& job) {
    m_queueMutex.lock();

    m_jobCount++;

    m_jobs.push(&job);        // Push job to queue
    m_condition.notify_one(); // Wake up a thread

    m_queueMutex.unlock();
}

void ThreadPool::run() {
    Arclight::Job* currentJob = nullptr;
    while (!m_threadsShouldDie && !m_jobs.empty()) {
        {
            std::unique_lock<std::mutex> acquiredLock(m_queueMutex);

            if (m_jobs.empty()) {
                continue;
            }

            currentJob = m_jobs.front();
            m_jobs.pop();
        }

        // Ensure we release lock before job is run
        currentJob->run();
        m_jobCount--;
    }
}

bool ThreadPool::Idle() const { return !m_jobCount; }

} // namespace Arclight
