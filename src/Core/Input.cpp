#include <Arclight/Core/Input.h>

#include <stdexcept>

namespace Arclight {

Input* Input::m_instance = nullptr;

Input::Input(){
    // TODO: For now it is a given that the singleton is initialzied before it is accessed by threads however we may want to consider thread-safety

    if(m_instance){
        throw std::runtime_error("Instance of Input already exists!");
    }

    m_instance = this;
}

Input::~Input(){
    m_instance = nullptr;
}

void Input::Tick(){
    for(auto& key : m_keyMap){
        // Make sure each tick we change pressed/released state to down/up
        if(key.second == KeyState_Pressed){
            key.second = KeyState_Down;
        } else if(key.second == KeyState_Released){
            key.second = KeyState_Up;
        }
    }
}

} // namespace Arclight