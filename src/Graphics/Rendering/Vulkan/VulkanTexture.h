#pragma once

#include <vulkan/vulkan_core.h>

#include <Arclight/Vector.h>

#include "VulkanMemory.h"

namespace Arclight::Rendering {

class VulkanTexture {
public:
	VulkanTexture(VulkanRenderer& renderer, const Vector2u& bounds);

private:
	Vector2u m_bounds;

	VkImage m_image;
	VmaAllocation m_alloaction;

	class VulkanRenderer& m_renderer;
};

} // namespace Arclight