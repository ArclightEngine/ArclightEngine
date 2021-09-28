#pragma once

#include <Arclight/Core/Logger.h>
#include <Arclight/Platform/Platform.h>

namespace {

template<typename... Args>
[[ noreturn ]] inline void _FatalRuntimeErrorImpl(Args... details){
    Arclight::Logger::Error(details...);
    abort();
}

}

#define FatalRuntimeError(...) _FatalRuntimeErrorImpl(__FILE__, "(", __LINE__, "): ", ##__VA_ARGS__)
