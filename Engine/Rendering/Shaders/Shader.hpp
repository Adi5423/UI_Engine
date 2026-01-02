#pragma once
#include <string>
#include <unordered_map>

#include <glm/glm.hpp>

class Shader
{
public:
    // Construct from vertex + fragment GLSL source strings
    Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
    ~Shader();

    void Bind() const;
    void Unbind() const;

    // Check if shader compiled and linked successfully
    bool IsValid() const { return m_RendererID != 0; }

    // Uniform helpers
    void SetMat4(const std::string& name, const glm::mat4& value);
    void SetFloat3(const std::string& name, const glm::vec3& value);
    void SetFloat4(const std::string& name, const glm::vec4& value);

private:
    uint32_t m_RendererID = 0;

    int GetUniformLocation(const std::string& name);

    std::unordered_map<std::string, int> m_UniformLocationCache;
};
