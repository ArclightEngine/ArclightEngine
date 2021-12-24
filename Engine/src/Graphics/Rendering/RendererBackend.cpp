#include <Arclight/Graphics/Rendering/Renderer.h>

#include <Arclight/Core/Job.h>
#include <Arclight/Core/ThreadPool.h>

#include <cassert>
namespace Arclight::Rendering {

void Renderer::render() {}

Renderer* Renderer::s_rendererInstance = nullptr;

} // namespace Arclight::Rendering