#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vulkan/vulkan.h>

#include <assert.h>

#include "Rendering/Vulkan.h"
#include "Window/WindowContext.h"

#include <vector>

int main(){
	assert(!SDL_Init(SDL_INIT_EVERYTHING));

	SDL_Window* win = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_VULKAN);
	assert(win);

	WindowContext windowContext(win);

	Rendering::VulkanRenderer vkRenderer;
	if(vkRenderer.Initialize(&windowContext)){
		return 1;
	}

	bool isRunning = true;
	while(isRunning){
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					isRunning = false;
					break;
				default:
					break;
			}
		}
	}

	return 0;
}