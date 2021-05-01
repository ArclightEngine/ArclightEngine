#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "Vector.h"
#include "Colour.h"

namespace Arclight {

class WindowContext final {
public:
	WindowContext(SDL_Window* window);

	inline SDL_Window* GetWindow() { return m_window; }

	// Returns a Vector2i containing the window size
	inline Vector2i GetSize() const { 
		Vector2i ret;
		
		SDL_GetWindowSize(m_window, &ret.x, &ret.y);

		return ret;
	}

	// Returns a Vector2i containing the window render size in pixels.
	// With some compositors, window size may not be in pixels 
	inline Vector2i GetWindowRenderSize() const {
		Vector2i ret;

		SDL_Vulkan_GetDrawableSize(m_window, &ret.x, &ret.y);

		return ret;
	}

	Colour backgroundColour = { 0, 0, 0, 255 };

private:
	SDL_Window* m_window;
};

} // namespace Arclight