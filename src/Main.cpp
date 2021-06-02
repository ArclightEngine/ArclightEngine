#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vulkan/vulkan.h>

#include <assert.h>
#include <unistd.h>

#include "Graphics/Rendering/Vulkan.h"
#include <Arclight/Window/WindowContext.h>
#include <Arclight/Graphics/Sprite.h>

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

	Image image;
	image.LoadResource("textures/art.png");

	Texture tex(image);
	Sprite spr(tex);
	
	vkRenderer.Draw(spr);
	vkRenderer.Render();

	signed rot = 0;
	while(isRunning){
		SDL_WaitEvent(nullptr);

		vkRenderer.Draw(spr);
		vkRenderer.Render();

		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					isRunning = false;
					break;
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_LEFT){
						rot -= 15;
					} else if(event.key.keysym.sym == SDLK_RIGHT){
						rot += 15;
					}
					break;
				default:
					break;
			}
		}
	}

	SDL_DestroyWindow(win);
	return 0;
}