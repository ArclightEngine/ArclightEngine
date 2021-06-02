#pragma once

namespace Arclight::Rendering {

class RenderObject {
public:
	virtual ~RenderObject() = default;

	virtual void Draw(class Renderer& renderer) = 0;
};

} // namespace Arclight::Rendering