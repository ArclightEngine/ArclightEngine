#include <Arclight/Platform/Platform.h>
#include <Arclight/Graphics/Rendering/Renderer.h>
#include <Arclight/Window/WindowContext.h>
#include <cassert>
#include <stdexcept>

#ifndef ARCLIGHT_SDL2
#error "No windowing library!"
#endif

#include <SDL2/SDL.h>

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

void Initialize(){
	int sdlError = SDL_Init(SDL_INIT_EVERYTHING);
    if(sdlError){
        throw std::runtime_error("Error Initializing SDL!");
        exit(1);
    }

	sdlWindow = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_VULKAN);
    if(!sdlWindow){
        throw std::runtime_error("Error creating SDL window!");
        exit(1);
    }

	windowContext = new WindowContext(sdlWindow);

#ifdef ARCLIGHT_VULKAN
    Rendering::VulkanRenderer* vkRenderer =  new Rendering::VulkanRenderer();
    if(!vkRenderer->Initialize(windowContext)){
        renderers.push_back(vkRenderer);
    }
#endif

#ifdef ARCLIGHT_DUMMY_RENDERER
    Rendering::DummyRenderer* dummyRenderer = new Rendering::DummyRenderer();
    if(!dummyRenderer->Initialize(windowContext)){
        renderers.push_back(dummyRenderer);
    }
#endif

    if(!renderers.size() || !Rendering::Renderer::Instance()){
        throw std::runtime_error("No available rendering API!");
        exit(2);
    }
}

void Cleanup(){
	SDL_DestroyWindow(sdlWindow);

    for(auto* renderer : renderers){
        delete renderer;
    }
    renderers.clear();
}

}