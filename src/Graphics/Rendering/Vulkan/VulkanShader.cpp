#include "VulkanShader.h"

#include <stdexcept>

namespace Arclight::Rendering {

VulkanShader::VulkanShader(ShaderStage stage, std::vector<uint8_t> data)
	: Shader(stage, data) {

}

VkShaderModule VulkanShader::CreateModule(VkDevice device){
	VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.codeSize = m_data.size(),
		.pCode = reinterpret_cast<const uint32_t*>(m_data.data()),
	};

	VkShaderModule sModule;
	if(vkCreateShaderModule(device, &createInfo, nullptr, &sModule)){
		throw std::runtime_error("[Fatal error] VulkanShader::CreateModule: Failed to create shader module!");
	}

	return sModule;
}

} // namespace Arclight::Rendering
