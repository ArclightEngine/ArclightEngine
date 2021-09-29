#pragma once

#include <Arclight/Core/Util.h>
#include <Arclight/Graphics/Rendering/Shader.h>

#include <GLES3/gl3.h>

#include <string>

#define SHADER_LOG_MAX_LENGTH 4096

namespace Arclight::Rendering {

class GLPipeline {
public:
    GLPipeline(const Shader& vertexShader, const Shader& fragmentShader);
    ~GLPipeline();

    ALWAYS_INLINE GLuint GetGLProgram() { return m_program; }
    ALWAYS_INLINE GLuint GetVAO() { return m_vao; }

private:
    static std::string GetShaderCompilationLog(GLuint shader); 
    static std::string GetProgramLinkLog(GLuint prog); 

    GLuint m_compiledVertexShader;
    GLuint m_compiledFragmentShader;

    GLuint m_program;

    // Vertex
    GLuint m_vao;
};

}