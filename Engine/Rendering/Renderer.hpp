#pragma once

#include <glm/glm.hpp>
#include <memory>

#include <Rendering/Mesh/Mesh.hpp>
#include <Rendering/Shaders/Shader.hpp>
#include <Rendering/Buffers/VertexArray.hpp>

class Renderer
{
public:
    static void Init();
    static void BeginScene(const glm::mat4& viewProj);
    static void EndScene();

    static void Clear(const glm::vec4& color);

    // Submit a mesh with a model transform and a shader
    static void Submit(const std::shared_ptr<Mesh>& mesh,
                       const glm::mat4& transform,
                       Shader& shader);

private:
    static glm::mat4 s_ViewProjection;
};
