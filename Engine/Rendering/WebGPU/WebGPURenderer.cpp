#include "WebGPURenderer.h"

#include <Arclight/Window/WindowContext.h>

#include <webgpu.h>

#include <SDL2/SDL_syswm.h>

namespace Arclight::Rendering {

WebGPURenderer::WebGPURenderer() {
    
}

int WebGPURenderer::initialize(class WindowContext* context) {
    s_rendererInstance = this;

    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(context->GetWindow(), &wmInfo);

#if defined(_WIN32)
    WGPUSurfaceDescriptorFromWindowsHWND nativeSurface = {};
    nativeSurface.hwnd = wmInfo.info.win.window;
    nativeSurface.hinstance = wmInfo.info.win.hinstance;
    nativeSurface.chain.sType = WGPUSType_SurfaceDescriptorFromWindowsHWND;
#elif defined(__linux__)
    // TODO: Wayland support

    WGPUSurfaceDescriptorFromXlib nativeSurface = {};
    nativeSurface.display = wmInfo.info.x11.display;
    nativeSurface.window = wmInfo.info.x11.window;
    nativeSurface.chain.sType = WGPUSType_SurfaceDescriptorFromXlib;
#elif defined(__EMSCRIPTEN__)
    WGPUSurfaceDescriptorFromCanvasHTMLSelector nativeSurface = {};
    nativeSurface.selector = "#canvas";
    nativeSurface.chain.sType = WGPUSType_SurfaceDescriptorFromCanvasHTMLSelector;
#endif

    WGPUSurfaceDescriptor surfaceDesc;
    surfaceDesc.nextInChain = &nativeSurface.chain;

    m_wgpuSurface = wgpuInstanceCreateSurface(NULL, &surfaceDesc);

    WGPUSwapChainDescriptor scDesc = {
        .nextInChain = {},
        .label = {},
        .usage = WGPUTextureUsage_RenderAttachment,
        .format = WGPUTextureFormat_BGRA8UnormSrgb,
        .width = context->get_size().x,
        .height = context->get_size().y,
        .presentMode = WGPUPresentMode_Fifo,
    };

    m_defaultPipeline = std::make_unique<RenderPipeline>(m_vertShader, m_fragShader);
    return 0;
}

}
