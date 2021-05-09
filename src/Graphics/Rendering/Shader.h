#pragma once

#include <vector>
#include <cstdint>

#include "NonCopyable.h"

namespace Arclight::Rendering {

class Shader
	: NonCopyable {
public:
	enum ShaderStage {
		InvalidShader,
		FragmentShader,
		VertexShader,
	};

	Shader(ShaderStage stage);
	Shader(ShaderStage stage, std::vector<uint8_t> data);

	inline ShaderStage GetStage() const { return m_stage; }
	inline const uint8_t* GetData() const { return m_data.data(); }
	inline size_t DataSize() const { return m_data.size(); }
	
protected:
	ShaderStage m_stage;
	std::vector<uint8_t> m_data; // e.g. SPIR-V for Vulkan
};

} // namespace Arclight::Rendering