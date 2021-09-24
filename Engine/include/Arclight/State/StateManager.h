#pragma once

#include <Arclight/Core/Util.h>
#include <Arclight/State/State.h>

#include <stack>

namespace Arclight {

class StateManager final {
public:
    StateManager();

    ALWAYS_INLINE bool IsEmpty() { return m_states.empty(); }
    ALWAYS_INLINE State CurrentState() { return m_states.top(); }

    // Push state to the stack
    ALWAYS_INLINE void Push(State state) { m_states.push(state); }
    // Pop state from the stack
    ALWAYS_INLINE void Pop() { m_states.pop(); };
    // Swap state on the top of the stack
    ALWAYS_INLINE void Swap(State state) {
        if (m_states.empty())
            m_states.push(state);
        else
            m_states.top() = state;
    }
    // Clear all states
    ALWAYS_INLINE void Clear() {
        while (m_states.size()) {
            m_states.pop();
        }
    };

private:
    std::stack<State> m_states;
};

} // namespace Arclight