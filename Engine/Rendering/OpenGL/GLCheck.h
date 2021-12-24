#pragma once

#include <GLES3/gl3.h>

#include <Arclight/Core/Fatal.h>

#include <cassert>
#include <string>

static constexpr inline const char* GLErrorString(GLuint error) {
    switch (error) {
    case GL_NO_ERROR:
        return "No error";
    case GL_INVALID_ENUM:
        return "Invalid enum";
    case GL_INVALID_VALUE:
        return "Invalid value";
    case GL_INVALID_OPERATION:
        return "Invalid operation";
    case GL_OUT_OF_MEMORY:
        return "Out of memory";
    case GL_INVALID_FRAMEBUFFER_OPERATION:
        return "Invalid framebuffer operation";
    default:
        return "Unknown error";
    }
}

/*Check for previous errors first*/
#define glCheck(x)                                                                                 \
    {                                                                                              \
        assert(glGetError() == GL_NO_ERROR);                                                       \
        (x);                                                                                       \
        if (auto e = glGetError(); e != GL_NO_ERROR)                                               \
            FatalRuntimeError("glCheck failed with error of {} ({})", GLErrorString(e), e);     \
    }
