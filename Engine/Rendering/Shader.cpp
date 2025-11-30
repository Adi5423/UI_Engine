#include "Shader.hpp"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

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
        char log[1024];
        glGetShaderInfoLog(id, 1024, nullptr, log);
        std::cout << "Shader compile error: " << log << std::endl;
    }

    return id;
}

Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc)
{
    uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    m_RendererID = glCreateProgram();
    glAttachShader(m_RendererID, vs);
    glAttachShader(m_RendererID, fs);
    glLinkProgram(m_RendererID);

    int success;
    glGetProgramiv(m_RendererID, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[1024];
        glGetProgramInfoLog(m_RendererID, 1024, nullptr, log);
        std::cout << "Shader link error: " << log << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
}

Shader::~Shader()
{
    glDeleteProgram(m_RendererID);
}

void Shader::Bind() const
{
    glUseProgram(m_RendererID);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

int Shader::GetUniformLocation(const std::string& name)
{
    auto it = m_UniformLocationCache.find(name);
    if (it != m_UniformLocationCache.end())
        return it->second;

    int loc = glGetUniformLocation(m_RendererID, name.c_str());
    m_UniformLocationCache[name] = loc;
    return loc;
}

void Shader::SetMat4(const std::string& name, const glm::mat4& value)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(value));
}

void Shader::SetFloat3(const std::string& name, const glm::vec3& value)
{
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}
