#pragma once

#include <vulkan/vulkan_core.h>

#define vkCheck(x)                                                                                 \
    {                                                                                              \
        VkResult r = (x);                                                                          \
        if (r != VK_SUCCESS)                                                                       \
            FatalRuntimeError("[Fatal error] {} vkCheck failed ({})!", __PRETTY_FUNCTION__, r);    \
    }

namespace Arclight::Rendering {

void VulkanImageLayoutTransition(VkCommandBuffer commandBuffer, VkImage image,
                                 VkImageLayout oldLayout, VkImageLayout newLayout);

} // namespace Arclight::Rendering
