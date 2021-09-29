#include "GLPipeline.h"

#include <Arclight/Core/Fatal.h>
#include <Arclight/Core/Logger.h>
#include <Arclight/Graphics/Vertex.h>

#include <cassert>

#include "GLCheck.h"

namespace Arclight::Rendering {

GLPipeline::GLPipeline(const Shader& vertexShader, const Shader& fragmentShader) {
    m_compiledVertexShader = glCreateShader(GL_VERTEX_SHADER);
    m_compiledFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    assert(m_compiledVertexShader && m_compiledFragmentShader);

    const GLchar* shaderSource = reinterpret_cast<const GLchar*>(vertexShader.GetData());
    GLint shaderSize = static_cast<GLint>(vertexShader.DataSize());
    glShaderSource(m_compiledVertexShader, 1, &shaderSource, &shaderSize);
    glCompileShader(m_compiledVertexShader);

    GLint compileStatus;
    glGetShaderiv(m_compiledVertexShader, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus) {
        FatalRuntimeError("GLPipeline: Failed to compile vertex shader. Log:\n",
                          GetShaderCompilationLog(m_compiledVertexShader));
    }

    shaderSource = reinterpret_cast<const GLchar*>(fragmentShader.GetData());
    shaderSize = static_cast<GLint>(fragmentShader.DataSize());
    glShaderSource(m_compiledFragmentShader, 1, &shaderSource, &shaderSize);
    glCompileShader(m_compiledFragmentShader);

    glGetShaderiv(m_compiledFragmentShader, GL_COMPILE_STATUS, &compileStatus);
    if (!compileStatus) {
        FatalRuntimeError("GLPipeline: Failed to compile fragment shader. Log:\n",
                          GetShaderCompilationLog(m_compiledFragmentShader));
    }

    m_program = glCreateProgram();
    assert(m_program);

    glCheck(glAttachShader(m_program, m_compiledVertexShader));
    glCheck(glAttachShader(m_program, m_compiledFragmentShader));

    glCheck(glLinkProgram(m_program));

    glGetProgramiv(m_program, GL_LINK_STATUS, &compileStatus);
    if (!compileStatus) {
        FatalRuntimeError("GLPipeline: Failed to link program. Log:\n",
                          GetProgramLinkLog(m_program));
    }

    glCheck(glDeleteShader(m_compiledVertexShader));
    glCheck(glDeleteShader(m_compiledFragmentShader));

    // Check for transform uniform block
    GLuint transformBlockIndex = glGetUniformBlockIndex(m_program, "Transform");
    if (transformBlockIndex != GL_INVALID_INDEX) {
        // Bind to Uniform Buffer Object 1
        // The shader will insert the relevant transform
        glCheck(glUniformBlockBinding(m_program, transformBlockIndex, 1));
    }

    m_modelTransformIndex = glGetUniformLocation(m_program, "transform");

    // Generate VAO for Pipeline
    glGenVertexArrays(1, &m_vao);
    assert(m_vao);

    // Define our vertex format
    glCheck(glBindVertexArray(m_vao));

    // Position
    /*glCheck(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), NULL));
    glCheck(glEnableVertexAttribArray(0));
    // Texture Coordinates
    glCheck(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (const void*)offsetof(Vertex, texCoord)));
    glCheck(glEnableVertexAttribArray(1));
    // Vertex Colour
    glCheck(glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (const void*)offsetof(Vertex, colour)));
    glCheck(glEnableVertexAttribArray(2));

    glCheck(glBindVertexArray(0));*/
}

GLPipeline::~GLPipeline() {
    glDeleteVertexArrays(1, &m_vao);
    
    glCheck(glDeleteProgram(m_program));

    glCheck(glDeleteShader(m_compiledVertexShader));
    glCheck(glDeleteShader(m_compiledFragmentShader));
}

std::string GLPipeline::GetShaderCompilationLog(GLuint shader) {
    GLsizei length = 0;
    std::string logString;
    logString.resize(SHADER_LOG_MAX_LENGTH);

    // Get shader log
    glGetShaderInfoLog(shader, SHADER_LOG_MAX_LENGTH, &length, logString.data());
    logString.resize(length); // Truncate to length

    return logString;
}

std::string GLPipeline::GetProgramLinkLog(GLuint shader) {
    GLsizei length = 0;
    std::string logString;
    logString.resize(SHADER_LOG_MAX_LENGTH);

    // Get program log
    glGetProgramInfoLog(shader, SHADER_LOG_MAX_LENGTH, &length, logString.data());
    logString.resize(length); // Truncate to length

    return logString;
}

} // namespace Arclight::Rendering
