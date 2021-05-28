#include "VulkanRenderer.h"
#include "VulkanTexture.h"
#include "VulkanMemory.h"
#include "VulkanPrivate.h"

#include <Arclight/Colour.h>

#include <stdexcept>

namespace Arclight::Rendering {

VulkanTexture::VulkanTexture(VulkanRenderer& renderer, const Vector2u& bounds)
    : m_renderer(renderer), m_bounds(bounds) {
    VkBufferCreateInfo stagingBufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = sizeof(RGBAColour) * bounds.x * bounds.y,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
    };

    VmaAllocationCreateInfo stagingBufferAllocCreateInfo = {
		.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = VMA_MEMORY_USAGE_CPU_ONLY,
		.requiredFlags = 0,
		.preferredFlags = 0,
		.memoryTypeBits = 0,
		.pool = 0,
		.pUserData = nullptr,
		.priority = 0.0f,
	};

    VmaAllocationInfo stagingAllocInfo;
    vkCheck(vmaCreateBuffer(m_renderer.Allocator(), &stagingBufferCreateInfo, &stagingBufferAllocCreateInfo, &m_staging, &m_stagingAllocation, &stagingAllocInfo));
    
    m_stagingMap = stagingAllocInfo.pMappedData;
    for(unsigned i = 0; i < bounds.x * bounds.y; i++){
        reinterpret_cast<uint32_t*>(m_stagingMap)[i] = 0xff00ffff; // Fill missing textures with pink
    }

    VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .extent = {
            m_bounds.x, // Width
            m_bounds.y, // Height
            1, // Depth
        },
        .mipLevels = 1, // No mipmaps
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT, // No multisampling
        .tiling = VK_IMAGE_TILING_OPTIMAL, // Most efficient, may not be row-major
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED, // Discard texels on image transition
    };

    VmaAllocationCreateInfo imageAllocCreateInfo = {
		.flags = 0,
		.usage = VMA_MEMORY_USAGE_GPU_ONLY,
		.requiredFlags = 0,
		.preferredFlags = 0,
		.memoryTypeBits = 0,
		.pool = 0,
		.pUserData = nullptr,
		.priority = 0.0f,
	};
    
    vkCheck(vmaCreateImage(m_renderer.Allocator(), &imageCreateInfo, &imageAllocCreateInfo, &m_image, &m_imageAllocation, nullptr));

    requireLayoutTransition = true;
    UpdateTextureImage();
}

VulkanTexture::~VulkanTexture(){
    vmaDestroyImage(m_renderer.Allocator(), m_image, m_imageAllocation);
    vmaDestroyBuffer(m_renderer.Allocator(), m_staging, m_stagingAllocation);
}

void VulkanTexture::LayoutTransition(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout){
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = 0,
        .dstAccessMask = 0,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = m_image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1, // We do not use mipmapping
            .baseArrayLayer = 0,
            .layerCount = 1, // The image is not an array
        },
    };

    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags destStage;

    if(newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL){
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if(newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL){ // Shaders should wait for transfer writes
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else {
        throw std::runtime_error("VulkanTexture::LayoutTransition: Invalid image layout!");
    }

    vkCmdPipelineBarrier(commandBuffer, srcStage, destStage, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier);
}

void VulkanTexture::UpdateTextureImage(){
    auto commandBuffer = m_renderer.CreateOneTimeCommandBuffer(); // Get a one-time command buffer for our copy operation

    VkBufferImageCopy copyInfo = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {
            m_bounds.x,
            m_bounds.y,
            1
        },
    };

    if(requireLayoutTransition){
        LayoutTransition(commandBuffer.Buffer(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        vkCmdCopyBufferToImage(commandBuffer.Buffer(), m_staging, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

        LayoutTransition(commandBuffer.Buffer(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        requireLayoutTransition = false;
    } else {
        vkCmdCopyBufferToImage(commandBuffer.Buffer(), m_staging, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);
    }
}

};