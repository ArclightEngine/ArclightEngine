#pragma once

#include <GLES3/gl3.h>

#include <cassert>
#include <string>

#define glCheck(x)                                                                                 \
    {                                                                                              \
        (x);                                                                                       \
        if (glGetError() != GL_NO_ERROR)                                                           \
            assert(!(std::string("[Fatal error]") + __PRETTY_FUNCTION__ + " glCheck failed!")      \
                        .c_str());                                                                 \
    }
