#pragma once

#include <Arclight/Core/Job.h>

#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <queue>
#include <condition_variable>

namespace Arclight {

class ThreadPool final {
    friend void ThreadMain(ThreadPool* pool);
public:
    ThreadPool();
    ~ThreadPool();

    void Schedule(Job& job);

    ////////////////////////////////////////
	/// \brief Execute ThreadPool job queue until it is exhausted
    ////////////////////////////////////////
    void run();

    ////////////////////////////////////////
	/// \brief Check if ThreadPool is idle.
    ///
    /// Returns true when the ThreadPool is idle.
    /// Meaning total number of jobs (running + queued) is zero.
    ///
    /// \return True if no running or queued jobs, otherwise false.
    ////////////////////////////////////////
    bool Idle() const;

    unsigned thread_count() const { return m_threads.size(); }
    unsigned job_count() const { return m_jobCount; }

    inline static ThreadPool* instance() { return m_instance; }

private:
    static ThreadPool* m_instance;

    bool m_threadsShouldDie = false;

    std::vector<std::thread> m_threads; // Thread objects

    std::atomic<unsigned> m_jobCount = 0; // Total amount of jobs (running and queued)
    std::queue<Job*> m_jobs; // FIFO Job Queue
    std::condition_variable m_condition; // Used to wait for available jobs
    std::mutex m_queueMutex; // Queue Mutex
};

} // namespace Arclight
