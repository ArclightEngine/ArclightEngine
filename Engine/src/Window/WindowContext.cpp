#include <Arclight/Window/WindowContext.h>

#include <Arclight/Core/Fatal.h>
#include <Arclight/Graphics/Rendering/Renderer.h>

namespace Arclight {

WindowContext* WindowContext::m_instance = nullptr;

WindowContext::WindowContext(SDL_Window* window) : m_window(window) { m_instance = this; }

WindowContext::~WindowContext() { m_instance = nullptr; }

void WindowContext::set_size(const Vector2i& size) {
    SDL_SetWindowSize(m_window, size.x, size.y);
    Rendering::Renderer::instance()->resize_viewport(size);
}

} // namespace Arclight
