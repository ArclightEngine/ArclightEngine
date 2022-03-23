#pragma once

#include <Arclight/Platform/Platform.h>

#ifdef ARCLIGHT_PLATFORM_WINDOWS

#ifdef ARCLIGHT_API_SHOULD_EXPORT
#define ARCLIGHT_API __declspec(dllexport)
#else
#define ARCLIGHT_API __declspec(dllimport)
#endif

#else
#define ARCLIGHT_API
#endif
