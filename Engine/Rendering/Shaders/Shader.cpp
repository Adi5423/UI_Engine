#include "Shader.hpp"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <Core/Log.hpp>
#include <Core/GLDebug.hpp>

// ============================================================================
// Shader Compilation - Professional Error Handling (Unity/Unreal Standard)
// ============================================================================
static uint32_t CompileShader(uint32_t type, const std::string& src)
{
    uint32_t id = glCreateShader(type);
    const char* csrc = src.c_str();
    glShaderSource(id, 1, &csrc, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        int length = 0;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        
        char* log = new char[length];
        glGetShaderInfoLog(id, length, &length, log);
        
        const char* shaderType = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
        CORE_ERROR("[Shader Compilation Failed] Type: {0}\n{1}", shaderType, log);
        
        delete[] log;
        glDeleteShader(id); // Delete invalid shader
        return 0; // Return 0 to indicate failure
    }

    return id;
}

Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc)
{
    uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    // Abort if shader compilation failed
    if (vs == 0 || fs == 0)
    {
        if (vs != 0) glDeleteShader(vs);
        if (fs != 0) glDeleteShader(fs);
        CORE_ERROR("[Shader Program] Failed to compile shaders - aborting program creation");
        m_RendererID = 0; // Mark as invalid
        return;
    }

    m_RendererID = glCreateProgram();
    glAttachShader(m_RendererID, vs);
    glAttachShader(m_RendererID, fs);
    glLinkProgram(m_RendererID);

    int success;
    glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
    if (!success)
    {
        int length = 0;
        glGetProgramiv(m_RendererID, GL_INFO_LOG_LENGTH, &length);
        
        char* log = new char[length];
        glGetProgramInfoLog(m_RendererID, length, &length, log);
        CORE_ERROR("[Shader Link Failed]\n{0}", log);
        delete[] log;
        
        glDeleteProgram(m_RendererID);
        m_RendererID = 0; // Mark as invalid
    }

    // Clean up shader objects (they're linked into program now)
    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader()
{
    if (m_RendererID != 0)
        glDeleteProgram(m_RendererID);
}

void Shader::Bind() const
{
    if (m_RendererID != 0)
        GL_CALL(glUseProgram(m_RendererID));
}

void Shader::Unbind() const
{
    GL_CALL(glUseProgram(0));
}

int Shader::GetUniformLocation(const std::string& name)
{
    if (m_RendererID == 0) return -1; // Invalid shader

    auto it = m_UniformLocationCache.find(name);
    if (it != m_UniformLocationCache.end())
        return it->second;

    int loc = glGetUniformLocation(m_RendererID, name.c_str());
    if (loc == -1)
        CORE_WARN("[Shader] Uniform '{0}' not found or unused", name);
        
    m_UniformLocationCache[name] = loc;
    return loc;
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value)
{
    int loc = GetUniformLocation(name);
    if (loc != -1)
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetFloat3(const std::string& name, const glm::vec3& value)
{
    int loc = GetUniformLocation(name);
    if (loc != -1)
        glUniform3f(loc, value.x, value.y, value.z);
}

void Shader::SetFloat4(const std::string& name, const glm::vec4& value)
{
    int loc = GetUniformLocation(name);
    if (loc != -1)
        glUniform4f(loc, value.x, value.y, value.z, value.w);
}
