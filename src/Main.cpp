#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vulkan/vulkan.h>

#include <assert.h>
#include <unistd.h>

#include "Graphics/Rendering/Vulkan.h"

#include <Arclight/Core/ThreadPool.h>

#include <Arclight/Window/WindowContext.h>
#include <Arclight/Graphics/Sprite.h>

#include <vector>

using namespace Arclight;

bool isRunning = true;

int main(){
	assert(!SDL_Init(SDL_INIT_EVERYTHING));

	SDL_Window* win = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_VULKAN);
	assert(win);

	ThreadPool threadPool;

	WindowContext windowContext(win);

	Rendering::VulkanRenderer vkRenderer;
	if(vkRenderer.Initialize(&windowContext)){
		return 1;
	}

	Image image;
	image.LoadResource("textures/art.png");

	Texture tex(image);
	Sprite spr(tex);

	spr.SetPosition(320, 240);
	spr.SetScale(0.25f, 0.25f);
	
	vkRenderer.Draw(spr);
	vkRenderer.Render();

	while(isRunning){
		SDL_WaitEventTimeout(nullptr, 10);

		vkRenderer.Render();

		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					isRunning = false;
					break;
				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_LEFT){
						spr.Move(-15, 0);
					} else if(event.key.keysym.sym == SDLK_RIGHT){
						spr.Move(15, 0);
					} else if(event.key.keysym.sym == SDLK_UP){
						spr.Move(0, -15);
					} else if(event.key.keysym.sym == SDLK_DOWN){
						spr.Move(0, 15);
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