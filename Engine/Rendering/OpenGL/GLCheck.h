#pragma once

#include <GLES3/gl3.h>

#include <Arclight/Core/Fatal.h>

#include <cassert>
#include <string>

/*Check for previous errors first*/
#define glCheck(x)                                                                                 \
    {                                                                                              \
        assert(glGetError() == GL_NO_ERROR);                                                       \
        (x);                                                                                       \
        if (auto e = glGetError(); e != GL_NO_ERROR)                                               \
            FatalRuntimeError("glCheck failed with error of ", e);                                 \
    }
