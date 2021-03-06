#include <Arclight/Core/Fatal.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Platform/Platform.h>
#include <Arclight/Window/WindowContext.h>

#include <cassert>

#include <SDL.h>
#include <SDL_video.h>

#ifdef ARCLIGHT_VULKAN
#include <SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <Rendering/Vulkan/VulkanRenderer.h>
#endif

#ifdef ARCLIGHT_DUMMY_RENDERER
#include <Rendering/Dummy/DummyRenderer.h>
#endif

#ifdef ARCLIGHT_OPENGL
#include <SDL2/SDL_opengles2.h>

#include <Rendering/OpenGL/GLRenderer.h>
#endif

#ifdef ARCLIGHT_WEBGPU
#include <Rendering/WebGPU/WebGPURenderer.h>
#endif

namespace Arclight::Platform {

std::vector<Rendering::Renderer*> renderers;
WindowContext* windowContext;
SDL_Window* sdlWindow;

// Emscripten does not have gamepad or haptic support
#ifdef ARCLIGHT_PLATFORM_WASM
const Uint32 platformSDLInitFlags =
    SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS;
#else
const Uint32 platformSDLInitFlags = SDL_INIT_EVERYTHING;
#endif

#if defined(ARCLIGHT_VULKAN)
const Uint32 platformSDLWindowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE;
#elif defined(ARCLIGHT_OPENGL)
const Uint32 platformSDLWindowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
#else
const Uint32 platformSDLWindowFlags = SDL_WINDOW_RESIZABLE;
#endif

void Initialize() {
    int sdlError = SDL_Init(platformSDLInitFlags);
    if (sdlError) {
        Logger::Error("Error \"{}\" Initializing SDL!", SDL_GetError());
        exit(1);
    }

    sdlWindow = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480,
                                 platformSDLWindowFlags);
    if (!sdlWindow) {
        Logger::Error("Error \"{}\" creating SDL window!", SDL_GetError());
        exit(1);
    }

    windowContext = new WindowContext(sdlWindow);

#ifdef ARCLIGHT_VULKAN
    Rendering::VulkanRenderer* vkRenderer = new Rendering::VulkanRenderer();
    if (!vkRenderer->initialize(windowContext)) {
        renderers.push_back(vkRenderer);
    }
#endif

#ifdef ARCLIGHT_DUMMY_RENDERER
    Rendering::DummyRenderer* dummyRenderer = new Rendering::DummyRenderer();
    if (!dummyRenderer->initialize(windowContext)) {
        renderers.push_back(dummyRenderer);
    }
#endif

#ifdef ARCLIGHT_OPENGL
    Rendering::GLRenderer* glRenderer = new Rendering::GLRenderer();
    if (!glRenderer->initialize(windowContext)) {
        renderers.push_back(glRenderer);
    }
#endif

#ifdef ARCLIGHT_WEBGPU
    Rendering::WebGPURenderer* wgpuRenderer = new Rendering::WebGPURenderer();
    if (!wgpuRenderer->initialize(windowContext)) {
        renderers.push_back(wgpuRenderer);
    }
#endif

    if (!renderers.size() || !Rendering::Renderer::instance()) {
        Logger::Error("No available rendering API!");
        exit(2);
    }
}

void Cleanup() {
    SDL_DestroyWindow(sdlWindow);

    for (auto* renderer : renderers) {
        delete renderer;
    }
    renderers.clear();
}

} // namespace Arclight::Platform