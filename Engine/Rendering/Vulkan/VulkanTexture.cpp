#include "VulkanTexture.h"
#include "VulkanMemory.h"
#include "VulkanPrivate.h"
#include "VulkanRenderer.h"

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
    vkCheck(vmaCreateBuffer(m_renderer.Allocator(), &stagingBufferCreateInfo,
                            &stagingBufferAllocCreateInfo, &m_staging, &m_stagingAllocation,
                            &stagingAllocInfo));

    m_stagingMap = stagingAllocInfo.pMappedData;
    for (unsigned i = 0; i < bounds.x * bounds.y; i++) {
        reinterpret_cast<uint32_t*>(m_stagingMap)[i] = __builtin_bswap32(
            RGBAColour(255, 255, 0, 255).value); // Fill missing textures with pink
    }

    VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = VK_FORMAT_R8G8B8A8_SRGB,
        .extent =
            {
                m_bounds.x, // Width
                m_bounds.y, // Height
                1,          // Depth
            },
        .mipLevels = 1, // No mipmaps
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,  // No multisampling
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

    vkCheck(vmaCreateImage(m_renderer.Allocator(), &imageCreateInfo, &imageAllocCreateInfo,
                           &m_image, &m_imageAllocation, nullptr));

    requireLayoutTransition = true;
    UpdateTextureImage();

    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = m_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = imageCreateInfo.format, // Use the same format
        .components =
            {
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
                VK_COMPONENT_SWIZZLE_IDENTITY,
            },
        .subresourceRange =
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
    };

    vkCheck(vkCreateImageView(m_renderer.GetDevice(), &imageViewCreateInfo, nullptr, &m_imageView));

    // TODO: Filter and address mode configuration
    VkSamplerCreateInfo samplerCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_NEAREST, // Magnification filter
        .minFilter = VK_FILTER_NEAREST, // Minificatioon filter
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT, // Repeat texture
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT, // Repeat texture
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT, // Repeat texture
        .mipLodBias = 0.f,
        .anisotropyEnable = VK_FALSE, // No ansiotropy
        .maxAnisotropy = 0.f,         // Ignored as we are not using it
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.f,
        .maxLod = 0.f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK, // Used when sampling beyond image, not
                                                         // awfully relevant to us with repeat
        .unnormalizedCoordinates = VK_FALSE,             // Use [0, 1) texel range
    };

    vkCheck(vkCreateSampler(m_renderer.GetDevice(), &samplerCreateInfo, nullptr, &m_texSampler));

    m_descriptorImageInfo = {
        .sampler = m_texSampler,
        .imageView = m_imageView,
        .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    };
}

VulkanTexture::~VulkanTexture() {
    // We need to wait for command buffer completion
    // TODO: Use another thread?
    m_renderer.WaitDeviceIdle();

    vkDestroySampler(m_renderer.GetDevice(), m_texSampler, nullptr);
    vkDestroyImageView(m_renderer.GetDevice(), m_imageView, nullptr);

    vmaDestroyImage(m_renderer.Allocator(), m_image, m_imageAllocation);
    vmaDestroyBuffer(m_renderer.Allocator(), m_staging, m_stagingAllocation);
}

void VulkanTexture::LayoutTransition(VkCommandBuffer commandBuffer, VkImageLayout oldLayout,
                                     VkImageLayout newLayout) {
    VulkanImageLayoutTransition(commandBuffer, m_image, oldLayout, newLayout);
}

void VulkanTexture::UpdateTextureBuffer(const void* data) {
    memcpy(m_stagingMap, data, sizeof(RGBAColour) * m_bounds.x * m_bounds.y);
}

void VulkanTexture::UpdateTextureImage() {
    auto commandBuffer =
        m_renderer
            .CreateOneTimeCommandBuffer(); // Get a one-time command buffer for our copy operation

    VkBufferImageCopy copyInfo = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource{
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {m_bounds.x, m_bounds.y, 1},
    };

    if (requireLayoutTransition) {
        LayoutTransition(commandBuffer.Buffer(), VK_IMAGE_LAYOUT_UNDEFINED,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        vkCmdCopyBufferToImage(commandBuffer.Buffer(), m_staging, m_image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

        LayoutTransition(commandBuffer.Buffer(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        requireLayoutTransition = false;
    } else {
        LayoutTransition(commandBuffer.Buffer(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                         VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        vkCmdCopyBufferToImage(commandBuffer.Buffer(), m_staging, m_image,
                               VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyInfo);

        LayoutTransition(commandBuffer.Buffer(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}

}; // namespace Arclight::Rendering