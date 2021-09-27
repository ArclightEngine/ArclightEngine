#include <Arclight/Core/Logger.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Platform/Platform.h>
#include <Arclight/Window/WindowContext.h>
#include <cassert>
#include <stdexcept>

#ifndef ARCLIGHT_SDL2
#error "No windowing library!"
#endif

#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>

#ifdef ARCLIGHT_VULKAN
#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.h>

#include <Rendering/Vulkan/VulkanRenderer.h>
#endif

#ifdef ARCLIGHT_DUMMY_RENDERER
#include <Rendering/Dummy/DummyRenderer.h>
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

#ifdef ARCLIGHT_VULKAN
const Uint32 platformSDLWindowFlags = SDL_WINDOW_VULKAN;
#else
const Uint32 platformSDLWindowFlags = 0;
#endif

void Initialize() {
    int sdlError = SDL_Init(platformSDLInitFlags);
    if (sdlError) {
        Logger::Error("Error \"", SDL_GetError(), "\" Initializing SDL!");
        exit(1);
    }

    sdlWindow = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480,
                                 platformSDLWindowFlags);
    if (!sdlWindow) {
        Logger::Error("Error \"", SDL_GetError(), "\" creating SDL window!");
        exit(1);
    }

    windowContext = new WindowContext(sdlWindow);

#ifdef ARCLIGHT_VULKAN
    Rendering::VulkanRenderer* vkRenderer = new Rendering::VulkanRenderer();
    if (!vkRenderer->Initialize(windowContext)) {
        renderers.push_back(vkRenderer);
    }
#endif

#ifdef ARCLIGHT_DUMMY_RENDERER
    Rendering::DummyRenderer* dummyRenderer = new Rendering::DummyRenderer();
    if (!dummyRenderer->Initialize(windowContext)) {
        renderers.push_back(dummyRenderer);
    }
#endif

    if (!renderers.size() || !Rendering::Renderer::Instance()) {
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