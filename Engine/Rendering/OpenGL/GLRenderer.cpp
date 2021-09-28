#include <Rendering/OpenGL/GLRenderer.h>

#include <Arclight/Window/WindowContext.h>
#include <Arclight/Platform/Platform.h>

#include <cassert>

namespace Arclight::Rendering {

int GLRenderer::Initialize(class WindowContext* context) {
    s_rendererInstance = this;


    m_defaultPipeline = std::make_unique<RenderPipeline>(m_vertShader, m_fragShader);
    return 0;
}

RenderPipeline::PipelineHandle
GLRenderer::CreatePipeline(const Shader&, const Shader&,
                           const RenderPipeline::PipelineFixedConfig&) {
    return nullptr;
}

void GLRenderer::DestroyPipeline(RenderPipeline::PipelineHandle) {
    
}

RenderPipeline& GLRenderer::DefaultPipeline() {
    assert(m_defaultPipeline.get());
    return *m_defaultPipeline;
}

Texture::TextureHandle GLRenderer::AllocateTexture(const Vector2u&) {
    return nullptr;
}

void GLRenderer::UpdateTexture(Texture::TextureHandle, const void*) {}

void GLRenderer::DestroyTexture(Texture::TextureHandle) {}

} // namespace Arclight::Rendering
