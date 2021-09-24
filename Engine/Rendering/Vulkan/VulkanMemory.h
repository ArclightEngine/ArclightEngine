#pragma once

#if defined(__GNUC__) // Clang and GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif

#if defined(__clang__) // Clang only
#pragma GCC diagnostic ignored "-Wnullability-completeness"
#endif

#include "VulkanMemoryAllocator/vk_mem_alloc.h"

#if defined(__clang__) || defined(__GNUC__)
#pragma GCC diagnostic pop
#endif
