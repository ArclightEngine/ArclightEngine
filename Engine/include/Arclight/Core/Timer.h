#pragma once

#include <chrono>

namespace Arclight {
    
class Timer final {
public:
    ////////////////////////////////////////
	/// \brief Get time elapsed in microseconds since last call
    ///
    /// \return Time in microseconds before last call
    ////////////////////////////////////////
    inline long elapsed(){
        auto newTimePoint = std::chrono::steady_clock::now();

        long elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(newTimePoint - m_timePoint).count();

        m_timePoint = newTimePoint;
        return elapsedUs;
    }

    inline void Reset(){
        m_timePoint = std::chrono::steady_clock::now();
    }

private:
    std::chrono::steady_clock::time_point m_timePoint = std::chrono::steady_clock::now();
};

} // namespace Arclight