#pragma once

#include <vector>
#include <cstdint>

namespace Arclight::Rendering {

class Shader {
public:
	enum ShaderStage {
		InvalidShader,
		FragmentShader,
		VertexShader,
	};

	inline ShaderStage GetStage() const { return m_stage; }
	inline const uint8_t* GetData() const { return m_data.data(); }
	
protected:
	Shader(ShaderStage stage);
	Shader(ShaderStage stage, std::vector<uint8_t> data);

	ShaderStage m_stage;
	std::vector<uint8_t> m_data; // e.g. SPIR-V for Vulkan
};

} // namespace Arclight::Rendering