#include "SceneRenderer.hpp"
#include <glad/glad.h>
#include <Rendering/Renderer.hpp>
#include <Scene/Components.hpp>
#include <Core/Log.hpp>

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
#version 410 core
layout(location = 0) in vec3 aPos;
uniform mat4 u_Model;
uniform mat4 u_ViewProj;
void main()
{
    gl_Position = u_ViewProj * u_Model * vec4(aPos, 1.0);
}
)";
    std::string fs = R"(
#version 410 core
out vec4 FragColor;
uniform vec4 u_Color;
void main()
{
    FragColor = u_Color;
}
)";
    m_Shader = std::make_shared<Shader>(vs, fs);
    
    // Check if shader is valid
    if (!m_Shader || !m_Shader->IsValid())
    {
        CORE_ERROR("[SceneRenderer] Failed to create shader! Viewport will be blank.");
    }
    else
    {
        CORE_INFO("[SceneRenderer] Shader compiled successfully.");
    }
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
    
    // Don't render if shader is invalid
    if (!m_Shader || !m_Shader->IsValid())
    {
        // Just clear the framebuffer
        m_Framebuffer->Bind();
        Renderer::Clear({ 0.12f, 0.12f, 0.14f, 1.0f });
        m_Framebuffer->Unbind();
        return;
    }

    m_Framebuffer->Bind();
    
    // 1. Clear Command
    glEnable(GL_DEPTH_TEST);
    Renderer::Clear({ 0.12f, 0.12f, 0.14f, 1.0f });

    // 2. Setup Scene Context
    m_Shader->Bind();
    m_Shader->SetMat4("u_ViewProj", camera.GetViewProjection());

    // DEBUG: Log rendering state (only once)
    static bool logged = false;
    if (!logged)
    {
        auto& reg = scene->Reg();
        int meshCount = 0;
        reg.view<TransformComponent, MeshComponent>().each([&](auto, auto&, auto&) { meshCount++; });
        
        CORE_INFO("[SceneRenderer DEBUG] Rendering {0} meshes", meshCount);
        CORE_INFO("[SceneRenderer DEBUG] Camera Position: ({0}, {1}, {2})", 
                  camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);
        CORE_INFO("[SceneRenderer DEBUG] Framebuffer: {0}x{1}", m_ViewportWidth, m_ViewportHeight);
        logged = true;
    }

    // 3. Render All Meshes
    auto& reg = scene->Reg();
    
    // Default blue-ish color for objects
    m_Shader->SetFloat4("u_Color", glm::vec4(0.2f, 0.7f, 1.0f, 1.0f));

    int renderedCount = 0;
    reg.view<TransformComponent, MeshComponent>().each([&](auto entity, TransformComponent& transform, MeshComponent& meshComp)
    {
        if (!meshComp.MeshHandle) return;
        
        m_Shader->SetMat4("u_Model", transform.GetMatrix());
        
        // This is a "Renderer::Submit" internal call effectively
        auto* va = meshComp.MeshHandle->GetVertexArray();
        va->Bind();
        glDrawElements(GL_TRIANGLES, meshComp.MeshHandle->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
        renderedCount++;
    });

    // DEBUG: Log if nothing was rendered
    static bool warnedOnce = false;
    if (renderedCount == 0 && !warnedOnce)
    {
        CORE_WARN("[SceneRenderer DEBUG] No meshes were rendered this frame!");
        warnedOnce = true;
    }

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
