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

    // BUG-009 FIX: Implement proper Blinn-Phong lighting shader
    // Professional game engines use lighting to provide depth perception
    std::string vs = R"(
#version 410 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

uniform mat4 u_Model;
uniform mat4 u_ViewProj;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    FragPos = vec3(u_Model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(u_Model))) * aNormal;
    gl_Position = u_ViewProj * vec4(FragPos, 1.0);
}
)";

    std::string fs = R"(
#version 410 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec4 u_Color;
uniform vec3 u_ViewPos;
uniform vec3 u_LightDir;

void main()
{
    // Ambient lighting
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * u_Color.rgb;
    
    // Diffuse lighting
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-u_LightDir);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * u_Color.rgb;
    
    // Specular lighting (Blinn-Phong)
    vec3 viewDir = normalize(u_ViewPos - FragPos);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
    vec3 specular = vec3(0.3) * spec;
    
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, u_Color.a);
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
        CORE_INFO("[SceneRenderer] Blinn-Phong lighting shader compiled successfully.");
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
    // FIX: Explicitly enable Depth Test to ensure correct Z-sorting
    // ImGui or other passes might have disabled it
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // FIX: Disable Face Culling to ensure planes are double-sided
    // This fixes "invisible planes" when looking from behind
    glDisable(GL_CULL_FACE);

    Renderer::Clear({ 0.12f, 0.12f, 0.14f, 1.0f });

    // 2. Setup Scene Context
    m_Shader->Bind();
    m_Shader->SetMat4("u_ViewProj", camera.GetViewProjection());
    
    // BUG-009: Set lighting uniforms for Blinn-Phong shader
    m_Shader->SetFloat3("u_ViewPos", camera.GetPosition());
    m_Shader->SetFloat3("u_LightDir", glm::vec3(-0.3f, -1.0f, -0.5f)); // Directional light from top-right

    // BUG-020 FIX: Removed static logging - allows dynamic debug output
    // DEBUG: Log rendering state can be re-enabled per-frame if needed for debugging

    // 3. Render All Meshes
    auto& reg = scene->Reg();
    
    // Default blue-ish color for objects
    m_Shader->SetFloat4("u_Color", glm::vec4(0.2f, 0.7f, 1.0f, 1.0f));

    int renderedCount = 0;
    reg.view<TransformComponent, MeshComponent>().each([&](auto entity, TransformComponent& transform, MeshComponent& meshComp)
    {
        if (!meshComp.MeshHandle) return;
        
        m_Shader->SetMat4("u_Model", transform.GetMatrix());
        
        // BUG-008 FIX: Add null check before dereferencing VA
        auto* va = meshComp.MeshHandle->GetVertexArray();
        if (!va) return;
        
        va->Bind();
        glDrawElements(GL_TRIANGLES, meshComp.MeshHandle->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
        renderedCount++;
    });

    // BUG-020 FIX: Removed static warning - allows repeated warnings if needed

    // 4. Render Selection Outline
    if (selectedEntity && selectedEntity.HasComponent<MeshComponent>())
    {
        auto& mc = selectedEntity.GetComponent<MeshComponent>();
        if (mc.MeshHandle)
        {
            auto& tc = selectedEntity.GetComponent<TransformComponent>();
            
            // BUG-026 FIX: Save current polygon mode
            GLint prevMode[2];
            glGetIntegerv(GL_POLYGON_MODE, prevMode);
            
            // BUG-010 FIX: Disable depth test so outline is always visible
            glDisable(GL_DEPTH_TEST);
            
            // Wireframe pass
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(4.0f); // BUG-024: May not work on all drivers (Core Profile limitation)
            
            m_Shader->SetFloat4("u_Color", glm::vec4(1.0f, 0.5f, 0.0f, 1.0f)); // Orange
            m_Shader->SetMat4("u_Model", tc.GetMatrix());
            
            // BUG-008 FIX: Add null check
            auto* va = mc.MeshHandle->GetVertexArray();
            if (va)
            {
                va->Bind();
                glDrawElements(GL_TRIANGLES, mc.MeshHandle->GetIndexCount(), GL_UNSIGNED_INT, nullptr);
            }
            
            // BUG-010 FIX: Re-enable depth test
            glEnable(GL_DEPTH_TEST);
            
            // BUG-026 FIX: Restore state properly
            glLineWidth(1.0f);
            glPolygonMode(GL_FRONT_AND_BACK, prevMode[0]);

            // HIGH-01 FIX: Restore default color so next frame starts clean
            m_Shader->SetFloat4("u_Color", glm::vec4(0.2f, 0.7f, 1.0f, 1.0f));
        }
    }

    // HIGH-01 FIX: Unbind shader and VAO to prevent state pollution into ImGui
    m_Shader->Unbind();
    glBindVertexArray(0);

    m_Framebuffer->Unbind();
}

uint32_t SceneRenderer::GetFinalImage()
{
    if (!m_Framebuffer) return 0;
    return m_Framebuffer->GetColorAttachment();
}
