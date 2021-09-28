#pragma once

#include <Arclight/Graphics/Rendering/Shader.h>

#include <GLES3/gl3.h>

namespace Arclight::Rendering {

class GLPipeline {
public:
    GLPipeline(const Shader& vertexShader, const Shader& fragmentShader);

private:
    GLuint m_compiledVertexShader;
    GLuint m_compiledFragmentShader;

    GLuint m_program;
};

}