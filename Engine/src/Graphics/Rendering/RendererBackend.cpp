#include <Arclight/Graphics/Rendering/Renderer.h>

#include <Arclight/Core/ThreadPool.h>
#include <Arclight/Core/Job.h>

#include <cassert>
#include <stdexcept>

namespace Arclight::Rendering {

void Renderer::Render(){
	
}

Renderer* Renderer::s_rendererInstance = nullptr;

} // namespace Rendering