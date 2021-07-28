#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include <vulkan/vulkan.h>

#include <assert.h>
#include <unistd.h>
#include <dlfcn.h>

#include "Graphics/Rendering/Vulkan.h"

#include <Arclight/Core/ThreadPool.h>

#include <Arclight/Window/WindowContext.h>
#include <Arclight/Graphics/Sprite.h>
#include <Arclight/Core/Scene.h>
#include <Arclight/Core/Input.h>
#include <Arclight/Core/Logger.h>

#include <vector>
#include <chrono>

using namespace Arclight;

bool isRunning = true;

class BouncingBox
	: public Node {
public:
	BouncingBox(const Sprite& spr, const Vector2f& pos)
		: m_sprite(spr) {
		m_sprite.SetPosition(pos);

		m_vector = { (((rand() % 2) << 1) - 1) * 120.f, (((rand() % 2) << 1) - 1) * 120.f };
	}

	void Tick(){
		m_sprite.Move(m_vector * Elapsed());

		Rectf bounds = m_sprite.Bounds();
		Vector2i winSize = WindowContext::Instance()->GetSize();
		if(bounds.bottom >= winSize.y){
			m_vector.y = -120;
		} else if(bounds.top <= 0){
			m_vector.y = 120;
		}

		if(bounds.right >= winSize.x){
			m_vector.x = -120;
		} else if(bounds.left <= 0){
			m_vector.x = 120;
		}
	}

	Vector2f m_vector = { 120, 120 };
	Sprite m_sprite;
};

class MoveableBox
	: public Node {
public:
	MoveableBox(const Sprite& spr, const Vector2f& pos)
		: m_sprite(spr) {
		m_sprite.SetPosition(pos);
	}

	void Tick(){
		if(Input::GetKeyDown((KeyCode)'w')){
			m_sprite.Move(0, -10 * Elapsed());
		}

		if(Input::GetKeyDown((KeyCode)'a')){
			m_sprite.Move(-10 * Elapsed(), 0);
		}

		if(Input::GetKeyDown((KeyCode)'s')){
			m_sprite.Move(0, 10 * Elapsed());
		}

		if(Input::GetKeyDown((KeyCode)'d')){
			m_sprite.Move(10 * Elapsed(), 0);
		}
	}

private:
	Sprite m_sprite;
};

class DVDSpawner
	: public Node {
public:
	DVDSpawner() {
		m_dvd.LoadResource("textures/dvd.png");

		m_tex.Load(m_dvd);
	}

	void Tick() override {
		if(Input::GetKeyPress(KeyCode_Space)){
			Sprite spr(m_tex);

			spr.SetPosition(0, 0);
			spr.SetScale(0.125f, 0.125f);

			AddChild(Node::Create<BouncingBox>(spr, Vector2f{rand() % WindowContext::Instance()->GetSize().x * 1.f, rand() % WindowContext::Instance()->GetSize().y * 1.f}));
			Logger::Debug(m_children.size(), " objects.");
		}

		if(Input::GetKeyPress(KeyCode_E)){
			Sprite spr(m_tex);

			spr.SetPosition(0, 0);
			spr.SetScale(0.125f, 0.125f);

			for(unsigned i = 100; i; i--){
				AddChild(Node::Create<BouncingBox>(spr, Vector2f{rand() % WindowContext::Instance()->GetSize().x * 1.f, rand() % WindowContext::Instance()->GetSize().y * 1.f}));
			}
			Logger::Debug(m_children.size(), " objects.");
		}

		if(Input::GetKeyPress(KeyCode_C)){
			m_children.clear();
		}
	}

private:
	Image m_dvd;
	Texture m_tex;
};

int main(){
	assert(!SDL_Init(SDL_INIT_EVERYTHING));

	SDL_Window* win = SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, SDL_WINDOW_VULKAN);
	assert(win);

	ThreadPool threadPool;
	Input inputManager;

	WindowContext windowContext(win);

	Rendering::VulkanRenderer vkRenderer;
	if(vkRenderer.Initialize(&windowContext)){
		return 1;
	}

	Image art;
	art.LoadResource("textures/art.png");

	Texture tex2(art);

	Timer timer;
	Scene scene(Node::Create<DVDSpawner>());

	auto pollEvents = [&]() -> void {
		SDL_Event event;
		while(SDL_PollEvent(&event)){
			switch(event.type){
				case SDL_QUIT:
					isRunning = false;
					break;
				case SDL_KEYDOWN:
					inputManager.OnKey((KeyCode)event.key.keysym.sym, Input::KeyState_Pressed);
					break;
				case SDL_KEYUP:
					inputManager.OnKey((KeyCode)event.key.keysym.sym, Input::KeyState_Released);
					break;
				default:
					break;
			}
		}
	};

	while(isRunning){
		inputManager.Tick();

		pollEvents();
		
		threadPool.Schedule(scene);
		vkRenderer.Render();

		while(!threadPool.Idle()) pollEvents(); // We shouldn't really busy wait

		long elapsed = timer.Elapsed();
		long waitTime = 1000000 / 60 - elapsed;
		if(waitTime > 0){
			//usleep(waitTime);
		}

		timer.Reset();
	}

	SDL_DestroyWindow(win);
	return 0;
}