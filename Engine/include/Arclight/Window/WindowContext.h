#pragma once

#include <SDL.h>
#include <SDL_vulkan.h>

#include <Arclight/Colour.h>
#include <Arclight/Core/UnicodeString.h>
#include <Arclight/Platform/API.h>
#include <Arclight/Vector.h>

namespace Arclight {

class ARCLIGHT_API WindowContext final {
public:
    WindowContext(SDL_Window* window);
    ~WindowContext();

    static inline WindowContext* instance() { return m_instance; }
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

    void SetSize(const Vector2i& size);

    void SetTitle(const UnicodeString& title){
		std::string utf8Title;
		title.toUTF8String(utf8Title);

        if (!utf8Title.empty()) {
            SDL_SetWindowTitle(m_window, utf8Title.c_str());
        }
	}

    Colour backgroundColour = {0, 0, 0, 255};

private:
    static WindowContext* m_instance;

    SDL_Window* m_window;
};

} // namespace Arclight