#include <Arclight/ECS/System.h>

#include <Arclight/Core/ThreadPool.h>

namespace Arclight {

System::System(std::string name)
    : m_name(std::move(name)) {
    
}

void System::Tick(){

}

void System::Run(){
    m_elapsedTime = m_timer.Elapsed() / 1000000.f;

    Tick();
}

} // namespace Arclight
