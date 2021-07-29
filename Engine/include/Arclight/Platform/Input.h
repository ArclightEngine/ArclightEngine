#pragma once

#ifdef ARCLIGHT_SDL2
    #include "SDL2/KeyCode.h"
#else
    #error "Failed to get input header for platform!" 
#endif