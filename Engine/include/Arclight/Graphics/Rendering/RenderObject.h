#pragma once

namespace Arclight::Rendering {

////////////////////////////////////////
/// \class RenderObject
///
/// Dervied classes are placed in render queue until destruction.
/// Draw function is called each render tick.
////////////////////////////////////////
class RenderObject {
public:
	RenderObject();
	RenderObject(const RenderObject&);
	RenderObject(RenderObject&&);

	RenderObject& operator=(RenderObject&&);
	RenderObject& operator=(const RenderObject&);

	virtual ~RenderObject();

	virtual void Draw() = 0;

protected:
	class Renderer& m_renderer;
};

} // namespace Arclight::Rendering