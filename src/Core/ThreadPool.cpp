#include <Arclight/Core/ThreadPool.h>

#include <Arclight/Core/Logger.h>

namespace Arclight {

void ThreadMain(ThreadPool* pool){
    while(!pool->m_threadsShouldDie){
        
    }
}

ThreadPool::ThreadPool(){
    // Hint of hardware threads
    unsigned cpuCount = std::thread::hardware_concurrency();
    if(cpuCount == 0){ // Failed to query hardware thread count
        cpuCount = 1;
    }

    Logger::Debug("[ThreadPool] Using ", cpuCount, " threads.");

    while(cpuCount--){
        m_threads.push_back(std::thread(ThreadMain, this));
    }
}

ThreadPool::~ThreadPool(){
    m_threadsShouldDie = true;
    
    while(!m_threads.empty()){
        for(auto& thread : m_threads){
            thread.join();
        }
    }
}

} // namespace Arclight