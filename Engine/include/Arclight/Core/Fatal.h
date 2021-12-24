#pragma once

#include <Arclight/Core/Logger.h>
#include <Arclight/Platform/Platform.h>

namespace {

template<typename... Args>
[[ noreturn ]] inline void _FatalRuntimeErrorImpl(const char* f, Args... details){
    Arclight::Logger::Error(f, details...);
    abort();
}

}

#define FatalRuntimeError(format, ...) _FatalRuntimeErrorImpl("{} ({}): " format, __FILE__, __LINE__, ##__VA_ARGS__)
