#include "SceneRenderer.hpp"
#include <glad/glad.h>
#include <Rendering/Renderer.hpp>
#include <Scene/Components.hpp>

SceneRenderer::SceneRenderer()
{
}

void SceneRenderer::Init()
{
    // Initialize Framebuffer
    m_Framebuffer = std::make_shared<Framebuffer>(m_ViewportWidth, m_ViewportHeight);

    // Initialize Shader (Basic Shader copied from EditorLayer)
    // In a real engine, this would be loaded from assets
    std::string vs = R"(
#version 450 core
layout(location = 0) in vec3 aPos;
uniform mat4 u_Model;
uniform mat4 u_ViewProj;
void main()
{
    gl_Position = u_ViewProj * u_Model * vec4(aPos, 1.0);
}
)";
    std::string fs = R"(
#version 450 core
out vec4 FragColor;
uniform vec4 u_Color;
void main()
{
    FragColor = u_Color;
}
)";
    m_Shader = std::make_shared<Shader>(vs, fs);
}

void SceneRenderer::SetViewportSize(uint32_t width, uint32_t height)
{
    if (m_ViewportWidth == width && m_ViewportHeight == height)
        return;

    m_ViewportWidth = width;
    m_ViewportHeight = height;

    if (m_Framebuffer)
        m_Framebuffer->Resize(width, height);
}

void SceneRenderer::RenderEditor(Scene* scene, const EditorCamera& camera, Entity selectedEntity)
{
    if (!m_Framebuffer || !scene) return;

    m_Framebuffer->Bind();
    
    // 1. Clear Command
    glEnable(GL_DEPTH_TEST);
    Renderer::Clear({ 0.12f, 0.12f, 0.14f, 1.0f });

    // 2. Setup Scene Context
    m_Shader->Bind();
    m_Shader->SetMat4("u_ViewProj", camera.GetViewProjection());

    // 3. Render All Meshes
    auto& reg = scene->Reg();
    
    // Default blue-ish color for objects
    m_Shader->SetFloat4("u_Color", glm::vec4(0.2f, 0.7f, 1.0f, 1.0f));

    reg.view<TransformComponent, MeshComponent>().each([&](auto entity, TransformComponent& transform, MeshComponent& meshComp)
    {
        if (!meshComp.MeshHandle) return;
        
        m_Shader->SetMat4("u_Model", transform.GetMatrix());
        
        // This is a "Renderer::Submit" internal call effectively
        auto* va = meshComp.MeshHandle->GetVertexArray();
        va->Bind();
        glDrawElements(GL_TRIANGLES, meshComp.MeshHandle->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
    });

    // 4. Render Selection Outline
    if (selectedEntity && selectedEntity.HasComponent<MeshComponent>())
    {
        auto& mc = selectedEntity.GetComponent<MeshComponent>();
        if (mc.MeshHandle)
        {
            auto& tc = selectedEntity.GetComponent<TransformComponent>();
            
            // Wireframe pass
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(4.0f);
            
            m_Shader->SetFloat4("u_Color", glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)); // Orange
            m_Shader->SetMat4("u_Model", tc.GetMatrix());
            
            auto* va = mc.MeshHandle->GetVertexArray();
            va->Bind();
            glDrawElements(GL_TRIANGLES, mc.MeshHandle->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
            
            // Restore state
            glLineWidth(1.0f);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    m_Framebuffer->Unbind();
}

uint32_t SceneRenderer::GetFinalImage()
{
    if (!m_Framebuffer) return 0;
    return m_Framebuffer->GetColorAttachment();
}
