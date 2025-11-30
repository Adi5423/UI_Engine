#include "EditorLayer.hpp"

#include <imgui.h>
#include <glad/glad.h>      // for glDrawElements etc.
#include <glm/glm.hpp>

EditorLayer::EditorLayer() = default;
EditorLayer::~EditorLayer() = default;

void EditorLayer::OnAttach()
{
    // Scene setup
    m_ActiveScene = std::make_unique<Scene>();

    {
        Entity camera = m_ActiveScene->CreateEntity("Camera");
        camera.AddComponent<TagComponent>("Camera");
        camera.AddComponent<TransformComponent>();

        Entity light = m_ActiveScene->CreateEntity("Directional Light");
        light.AddComponent<TagComponent>("Directional Light");
        light.AddComponent<TransformComponent>();

        Entity cube = m_ActiveScene->CreateEntity("Cube");
        cube.AddComponent<TagComponent>("Cube");
        cube.AddComponent<TransformComponent>();

        Entity floor = m_ActiveScene->CreateEntity("Floor");
        floor.AddComponent<TagComponent>("Floor");
        floor.AddComponent<TransformComponent>();
    }

    // Framebuffer
    m_Framebuffer = std::make_unique<Framebuffer>(1280, 720);

    // ---------- Simple cube geometry ----------
    float vertices[] = {
        // positions (a simple quad in front for now)
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
    };

    uint32_t indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    m_CubeVA = std::make_unique<VertexArray>();
    auto vb = new VertexBuffer(vertices, sizeof(vertices));
    auto ib = new IndexBuffer(indices, 6);
    m_CubeVA->AddVertexBuffer(vb);
    m_CubeVA->SetIndexBuffer(ib);

    // ---------- Simple shader ----------
    std::string vs = R"(
#version 450 core

layout(location = 0) in vec3 aPos;

uniform mat4 u_ViewProj;

void main()
{
    gl_Position = u_ViewProj * vec4(aPos, 1.0);
}
)";

    std::string fs = R"(
#version 450 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(0.2, 0.7, 1.0, 1.0);
}
)";

    m_Shader = std::make_unique<Shader>(vs, fs);

    // Renderer global init
    Renderer::Init();

    // Initial camera aspect
    m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
}

void EditorLayer::OnDetach()
{
    m_ActiveScene.reset();
    m_Framebuffer.reset();
    m_CubeVA.reset();
    m_Shader.reset();
}

void EditorLayer::OnImGuiRender()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

    DrawHierarchyPanel();
    DrawInspectorPanel();
    DrawContentBrowserPanel();
    DrawViewportPanel();

    ImGui::PopStyleVar(2);
}

// ------------------------- PANELS -------------------------

void EditorLayer::DrawHierarchyPanel()
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("Hierarchy", nullptr, flags))
    {
        ImGui::TextDisabled("Scene");
        ImGui::Separator();
        ImGui::Spacing();

        if (!m_ActiveScene)
        {
            ImGui::Text("No active scene.");
            ImGui::End();
            return;
        }

        auto& reg = m_ActiveScene->Reg();
        reg.view<TagComponent>().each([&](auto entityHandle, TagComponent& tag)
            {
                Entity entity(entityHandle, m_ActiveScene.get());
                bool isSelected = (m_SelectedEntity.Handle() == entityHandle);

                ImGuiTreeNodeFlags nodeFlags =
                    ImGuiTreeNodeFlags_Leaf |
                    ImGuiTreeNodeFlags_NoTreePushOnOpen |
                    ImGuiTreeNodeFlags_SpanAvailWidth;

                if (isSelected)
                    nodeFlags |= ImGuiTreeNodeFlags_Selected;

                ImGui::TreeNodeEx((void*)(uint64_t)entityHandle, nodeFlags, "%s", tag.Tag.c_str());

                if (ImGui::IsItemClicked())
                    m_SelectedEntity = entity;
            });
    }
    ImGui::End();
}

void EditorLayer::DrawInspectorPanel()
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("Inspector", nullptr, flags))
    {
        ImGui::TextDisabled("Inspector");
        ImGui::Separator();
        ImGui::Spacing();

        if (!m_SelectedEntity || !m_ActiveScene)
        {
            ImGui::Text("No entity selected.");
            ImGui::End();
            return;
        }

        auto& reg = m_ActiveScene->Reg();
        entt::entity handle = m_SelectedEntity.Handle();

        // Tag (name)
        if (reg.any_of<TagComponent>(handle))
        {
            auto& tag = reg.get<TagComponent>(handle);

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy(buffer, tag.Tag.c_str(), sizeof(buffer) - 1);

            ImGui::Text("Name");
            ImGui::SameLine();
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
                tag.Tag = std::string(buffer);

            ImGui::Separator();
        }

        // Transform
        if (reg.any_of<TransformComponent>(handle))
        {
            auto& transform = reg.get<TransformComponent>(handle);

            ImGui::Text("Transform");
            ImGui::Separator();

            ImGui::Text("Position");
            ImGui::SameLine();
            ImGui::DragFloat3("##Position", &transform.Position.x, 0.1f);

            ImGui::Text("Rotation");
            ImGui::SameLine();
            ImGui::DragFloat3("##Rotation", &transform.Rotation.x, 0.1f);

            ImGui::Text("Scale");
            ImGui::SameLine();
            ImGui::DragFloat3("##Scale", &transform.Scale.x, 0.1f);
        }
    }
    ImGui::End();
}

void EditorLayer::DrawContentBrowserPanel()
{
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus;

    if (ImGui::Begin("Content Browser", nullptr, flags))
    {
        ImGui::TextDisabled("Assets");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::Selectable("Meshes/")) {}
        if (ImGui::Selectable("Textures/")) {}
        if (ImGui::Selectable("Scenes/")) {}
    }
    ImGui::End();
}

void EditorLayer::DrawViewportPanel()
{
    ImGuiWindowFlags viewportFlags =
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse;

    if (ImGui::Begin("Viewport", nullptr, viewportFlags))
    {
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

        if ((uint32_t)m_ViewportSize.x > 0 &&
            (uint32_t)m_ViewportSize.y > 0 &&
            (m_Framebuffer->GetWidth() != (uint32_t)m_ViewportSize.x ||
                m_Framebuffer->GetHeight() != (uint32_t)m_ViewportSize.y))
        {
            m_Framebuffer->Resize(
                (uint32_t)m_ViewportSize.x,
                (uint32_t)m_ViewportSize.y
            );

            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        }

        // --- Render into framebuffer ---
        m_Framebuffer->Bind();
        Renderer::Clear({ 0.12f, 0.12f, 0.14f, 1.0f });

        m_Shader->Bind();
        m_Shader->SetMat4("u_ViewProj", m_EditorCamera.GetViewProjection());

        m_CubeVA->Bind();
        glDrawElements(GL_TRIANGLES,
            m_CubeVA->GetIndexBuffer()->GetCount(),
            GL_UNSIGNED_INT,
            nullptr);

        m_Framebuffer->Unbind();
        // -------------------------------

        uint32_t textureID = m_Framebuffer->GetColorAttachment();
        ImGui::Image((void*)(intptr_t)textureID,
            viewportPanelSize,
            ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();
}
