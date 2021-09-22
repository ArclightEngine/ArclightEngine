#pragma once

#include <vulkan/vulkan_core.h>

#include <cassert>

#define vkCheck(x)                                                                                 \
    {                                                                                              \
        VkResult r = (x);                                                                          \
        if (r != VK_SUCCESS)                                                                       \
            assert(!(std::string("[Fatal error]") + __PRETTY_FUNCTION__ + " vkCheck failed!")      \
                        .c_str());                                                                 \
    }
