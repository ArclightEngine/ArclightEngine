#pragma once

#include <string>

#include "Pipeline.h"

namespace Arclight::Rendering {

class Renderer {
public:
	virtual ~Renderer() = default;

	virtual int Initialize(class WindowContext* context) = 0;
	static inline Renderer* Instance() { return s_rendererInstance; }

	virtual void Draw() = 0;
	virtual void WaitDeviceIdle() const = 0;

	//virtual int LoadShader(const std::string& name) = 0;
	//virtual RenderPipeline* CreatePipeline(Shader* vertexShader, Shader* fragmentShader) = 0;

	virtual const std::string& GetName() const = 0;

protected:
	static Renderer* s_rendererInstance;
};

} // namespace Arclight::Rendering