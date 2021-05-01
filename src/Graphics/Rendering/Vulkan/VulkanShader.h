#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "Graphics/Rendering/ShaderBase.h"

namespace Arclight::Rendering {

class VulkanShader
	: public Shader {
public:
	VulkanShader(ShaderStage stage, std::vector<uint8_t> data);

	VkShaderModule CreateModule(VkDevice device);
private:
	
};

} // namespace Arclight::Rendering