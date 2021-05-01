#include "ShaderBase.h"

namespace Arclight::Rendering {

Shader::Shader(ShaderStage stage)
	: m_stage(stage) {
	
}

Shader::Shader(ShaderStage stage, std::vector<uint8_t> data)
	: m_stage(stage), m_data(data) {
	
}

} // namespace Arclight::Rendering