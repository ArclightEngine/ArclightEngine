#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vulkan/vulkan.h>

#include <assert.h>
#include <unistd.h>

#include "Graphics/Rendering/Vulkan.h"
#include <Arclight/Window/WindowContext.h>

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
		{{0, 50}, {0.0f, 1.0f}, {1.0f, 0.0f, 1.0f, 1.f}}, // Bottom left
		{{0, 0}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.f}}, // Top left
		{{50, 50}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.f}}, // Bottom right
		{{50, 0}, {1.0f, 0.0f}, {1.0f, 1.0f, 0.0f, 1.f}}, // Top right
	};

	Image image;
	image.LoadResource("textures/art.png");

	Texture tex(image);

	Transform transform;
	
	vkRenderer.Draw(vertices, 4, transform.Matrix(), tex.Handle());
	vkRenderer.Render();

	signed rot = 0;
	while(isRunning){
		SDL_WaitEvent(nullptr);

		transform.SetRotation(rot % 360);
		vkRenderer.Draw(vertices, 4, transform.Matrix(), tex.Handle());
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