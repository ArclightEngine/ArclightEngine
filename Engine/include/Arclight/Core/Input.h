#pragma once

#include <map>
#include <unordered_map>
#include <string>

#include <Arclight/Platform/Input.h>

#include <Arclight/Core/InputBinding.h>
#include <Arclight/Platform/API.h>

namespace Arclight {

class ARCLIGHT_API Input final {
public:
    enum KeyState {
        KeyState_Up = 0, // Key is up
        KeyState_Pressed = 1, // Key was pressed this tick
        KeyState_Released = 2, // Key was released this tick
        KeyState_Down = 3, // Key is down
    }; // If the key is down KeyState & 0x1 will be 1

    Input();
    ~Input();

    inline Input* instance() { return m_instance; }

    static inline KeyState GetKey(KeyCode key) { return m_instance->m_keyMap[key]; }
    static inline int GetKeyDown(KeyCode key) { return m_instance->m_keyMap[key] & 0x1; }
    static inline int GetKeyPress(KeyCode key) { return m_instance->m_keyMap[key] == KeyState_Pressed; }
    static inline int GetKeyRelease(KeyCode key) { return m_instance->m_keyMap[key] == KeyState_Released; }

    inline void OnKey(KeyCode key, KeyState state) { 
        // Prevent key repeat
        if(state == KeyState_Pressed && m_keyMap[key] != KeyState_Down){
            m_keyMap[key] = state;
        } else {
            m_keyMap[key] = state;
        }
    }

    void Tick();

private:
    static Input* m_instance; // We only need one Input instance

    std::unordered_map<KeyCode, KeyState> m_keyMap;
    std::unordered_map<std::string, InputBinding> m_bindings;
};

}; // namespace Arclight