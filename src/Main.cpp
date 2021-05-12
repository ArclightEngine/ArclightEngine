#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vulkan/vulkan.h>

#include <assert.h>
#include <unistd.h>

#include "Graphics/Rendering/Vulkan.h"
#include "Window/WindowContext.h"

#include <vector>

using namespace Arclight;

bool isRunning = true;

int main(){
	assert(!SDL_Init(SDL_INIT_EVERYTHING));

	SDL_Window* win = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_VULKAN);
	assert(win);

	WindowContext windowContext(win);

	Rendering::VulkanRenderer vkRenderer;
	if(vkRenderer.Initialize(&windowContext)){
		return 1;
	}

	Vertex vertices[] = {
		{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f, 1.f}},
		{{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.f}},
		{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f, 1.f}},
		{{0.5f, -0.5f}, {1.0f, 0.0f, 0.0f, 1.f}},
	};

	Transform transform;
	vkRenderer.Draw(vertices, 4, transform.Matrix());
	vkRenderer.Render();

	while(isRunning){
		vkRenderer.Draw(vertices, 4, transform.Matrix());
		vkRenderer.Render();

		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					isRunning = false;
					break;
				case SDL_KEYDOWN:
					break;
				default:
					break;
			}
		}
	}

	SDL_DestroyWindow(win);
	return 0;
}