#pragma once

#include <GLES3/gl3.h>

#include <Arclight/Core/Fatal.h>

#include <cassert>
#include <string>

#define glCheck(x)                                                                                 \
    {                                                                                              \
        (x);                                                                                       \
        if (auto e = glGetError(); e != GL_NO_ERROR)                                               \
            FatalRuntimeError("glCheck failed with error of ", e);                    \
    }
