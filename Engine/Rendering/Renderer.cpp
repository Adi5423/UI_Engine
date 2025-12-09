#include "Renderer.hpp"
#include <glad/glad.h>

glm::mat4 Renderer::s_ViewProjection{ 1.0f };

void Renderer::Init()
{
    glEnable(GL_DEPTH_TEST);
}

void Renderer::BeginScene(const glm::mat4& viewProj)
{
    s_ViewProjection = viewProj;
}

void Renderer::EndScene()
{
    // For now, nothing to do. Later: flush draw queues, etc.
}

void Renderer::Clear(const glm::vec4& color)
{
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::Submit(const std::shared_ptr<Mesh>& mesh,
                      const glm::mat4& transform,
                      Shader& shader)
{
    if (!mesh || !mesh->GetVertexArray())
        return;

    shader.Bind();
    shader.SetMat4("u_ViewProj", s_ViewProjection);
    shader.SetMat4("u_Model", transform);

    VertexArray* va = mesh->GetVertexArray();
    va->Bind();

    glDrawElements(GL_TRIANGLES,
                   mesh->GetIndexCount(),
                   GL_UNSIGNED_INT,
                   nullptr);
}
