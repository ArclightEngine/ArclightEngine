#pragma once

#include <string>

namespace Rendering {

class RendererBackend {
public:
	virtual ~RendererBackend() = default;

	virtual int Initialize(class WindowContext* context) = 0;
	static inline RendererBackend* Instance() { return s_rendererInstance; }

	virtual const std::string& GetName() const = 0;

protected:
	static RendererBackend* s_rendererInstance;
};

} // namespace Render