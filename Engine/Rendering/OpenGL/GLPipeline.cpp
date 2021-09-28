#include "GLPipeline.h"

#include <Arclight/Core/Logger.h>
#include <Arclight/Core/Fatal.h>

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
    if(!compileStatus){
        FatalRuntimeError("GLPipeline: Failed to compile vertex shader. Log:\n", GetShaderCompilationLog(m_compiledVertexShader));
    }

    shaderSource = reinterpret_cast<const GLchar*>(fragmentShader.GetData());
    shaderSize = static_cast<GLint>(fragmentShader.DataSize());
    glShaderSource(m_compiledFragmentShader, 1, &shaderSource, &shaderSize);
    glCompileShader(m_compiledFragmentShader);

    glGetShaderiv(m_compiledFragmentShader, GL_COMPILE_STATUS, &compileStatus);
    if(!compileStatus){
        FatalRuntimeError("GLPipeline: Failed to compile fragment shader. Log:\n", GetShaderCompilationLog(m_compiledFragmentShader));
    }

    m_program = glCreateProgram();
    assert(m_program);

    glCheck(glAttachShader(m_program, m_compiledVertexShader));
    glCheck(glAttachShader(m_program, m_compiledFragmentShader));

    glCheck(glLinkProgram(m_program));
}

GLPipeline::~GLPipeline(){
    glCheck(glDeleteProgram(m_program));

    glCheck(glDeleteShader(m_compiledVertexShader));
    glCheck(glDeleteShader(m_compiledFragmentShader));
}

std::string GLPipeline::GetShaderCompilationLog(GLuint shader){
    GLsizei length = 0;
    std::string logString;
    logString.resize(SHADER_LOG_MAX_LENGTH);

    // Get shader log
    glGetShaderInfoLog(shader, SHADER_LOG_MAX_LENGTH, &length, logString.data());
    logString.resize(length); // Truncate to length

    return logString;
}

} // namespace Arclight::Rendering
