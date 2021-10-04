#pragma once

#include <vulkan/vulkan_core.h>

#include <Arclight/Vector.h>

#include "VulkanMemory.h"

namespace Arclight::Rendering {

class VulkanTexture final {
public:
	VulkanTexture(class VulkanRenderer& renderer, const Vector2u& bounds, VkFormat texFormat);
	~VulkanTexture();

	////////////////////////////////////////
	/// Returns the staging buffer mapping for the texture.
	///
	/// \return Pointer to mapped staging buffer
	///
	/// \note This buffer is CPU-only. Call UpdateTextureImage to update the GPU copy.
	////////////////////////////////////////
	inline void* Buffer() { return m_stagingMap; }

	////////////////////////////////////////
	/// Update the staging buffer
	///
	/// \param data New pixel data
	////////////////////////////////////////
	void UpdateTextureBuffer(const void* data);

	////////////////////////////////////////
	/// Update the VkImage for a texture using the staging buffer.
	////////////////////////////////////////
	void UpdateTextureImage();

	inline const VkDescriptorImageInfo& DescriptorImageInfo() const { return m_descriptorImageInfo; }; // Used to update descriptor sets for the fragment shader

private:
	////////////////////////////////////////
	/// Transition the image layout using an image memory barrier.
	///
	/// \param commandBuffer Command buffer
	/// \param oldLayout Old image layout
	/// \param newLayout New image layout
	////////////////////////////////////////
	void LayoutTransition(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);

	class VulkanRenderer& m_renderer; // Renderer object

	Vector2u m_bounds; // Texture bounds

	bool requireLayoutTransition = true; // Layout transition when updating texture?

	void* m_stagingMap; // Staging buffer host mapping

	VkBuffer m_staging; // Staging buffer (CPU to GPU)
	VmaAllocation m_stagingAllocation;

	VkFormat m_format; // Texture format

	VkImage m_image; // Vulkan image object (GPU only, more efficient)
	VmaAllocation m_imageAllocation;
	VkDescriptorImageInfo m_descriptorImageInfo; // Used to update descriptor sets for the fragment shader

	VkImageView m_imageView; // Needs an image view
	VkSampler m_texSampler; // Image sampler (allows us to apply filters, etc.)

	VkDescriptorSet m_textureDescriptorSet; // Descriptor set for the texture
};

} // namespace Arclight