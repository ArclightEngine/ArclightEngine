#include "VulkanPrivate.h"

#include <Arclight/Core/Fatal.h>

namespace Arclight::Rendering {

void VulkanImageLayoutTransition(VkCommandBuffer commandBuffer, VkImage image,
                                 VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = 0,
        .dstAccessMask = 0,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1, // We do not use mipmapping
                .baseArrayLayer = 0,
                .layerCount = 1, // The image is not an array
            },
    };

    VkPipelineStageFlags srcStage = 0;
    VkPipelineStageFlags destStage = 0;

    if (newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) { // Shaders should wait for
                                                                        // transfer writes
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        // TODO: May need to add stencil bit

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        FatalRuntimeError("VulkanImageLayoutTransition: Invalid image layout!");
    }

    vkCmdPipelineBarrier(commandBuffer, srcStage, destStage, 0, 0, nullptr, 0, nullptr, 1,
                         &barrier);
}

} // namespace Arclight::Rendering
