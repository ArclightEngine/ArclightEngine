#include <Arclight/Window/WindowContext.h>

#include <stdexcept>

namespace Arclight {

WindowContext* WindowContext::m_instance = nullptr;

WindowContext::WindowContext(SDL_Window* window)
	: m_window(window) {
	if(m_instance){
		std::runtime_error("WindowContext instance already exists!");
	}

	m_instance = this;
}

WindowContext::~WindowContext(){
	m_instance = nullptr;
}

} // namespace Arclight