#include <Arclight/Core/Node.h>

#include <Arclight/Core/ThreadPool.h>

namespace Arclight {

Node::Node(){
    
}

void Node::Start(){
    for(auto& child : m_children){
        child->Start();
    }
}

void Node::Tick(){

}

void Node::Run(){
    m_elapsedTime = m_timer.Elapsed() / 1000000.f;

    for(auto& child : m_children){
        ThreadPool::Instance()->Schedule(*child);
    }

    Tick();
}

} // namespace Arclight