#include "GLPipeline.h"

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
    glCheck(glCompileShader(m_compiledVertexShader));

    shaderSource = reinterpret_cast<const GLchar*>(fragmentShader.GetData());
    shaderSize = static_cast<GLint>(fragmentShader.DataSize());
    glShaderSource(m_compiledFragmentShader, 1, &shaderSource, &shaderSize);
    glCheck(glCompileShader(m_compiledFragmentShader));

    m_program = glCreateProgram();
    assert(m_program);

    glCheck(glAttachShader(m_program, m_compiledVertexShader));
    glCheck(glAttachShader(m_program, m_compiledFragmentShader));

    glLinkProgram(m_program);
}

} // namespace Arclight::Rendering
