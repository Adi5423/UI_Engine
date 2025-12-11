#include "EditorLayer.hpp"

#include <imgui.h>
#include <imgui_internal.h>
#include <glad/glad.h>      // for glDrawElements etc.
#include <glm/glm.hpp>

#include <Core/Input/ViewportInput.hpp>
#include <Scene/Scene.hpp>
#include <Scene/Entity.hpp>
#include <Scene/Components.hpp>
#include <Scene/SceneAPI.hpp>
#include <Core/ThemeSettings.hpp>
#include <Core/ImGuiLayer.hpp>


EditorLayer::EditorLayer() = default;
EditorLayer::~EditorLayer() = default;

void EditorLayer::OnAttach()
{
    // Scene setup
    m_ActiveScene = std::make_unique<Scene>();

    // inspector pannels views.
    {
        // Scene setup via API
        m_ActiveScene = std::make_unique<Scene>();
        SceneAPI::CreateDefaultScene(*m_ActiveScene);

    }

    // Framebuffer
    m_Framebuffer = std::make_unique<Framebuffer>(1280, 720);

    // // ---------- Simple cube geometry ----------
    // float vertices[] = {
    //     // positions (a simple quad in front for now)
    //     -0.5f, -0.5f, -0.5f,
    //      0.5f, -0.5f, -0.5f,
    //      0.5f,  0.5f, -0.5f,
    //     -0.5f,  0.5f, -0.5f,
    // };

    // uint32_t indices[] = {
    //     0, 1, 2,
    //     2, 3, 0
    // };

    // m_CubeVA = std::make_unique<VertexArray>();
    // auto vb = new VertexBuffer(vertices, sizeof(vertices));
    // auto ib = new IndexBuffer(indices, 6);
    // m_CubeVA->AddVertexBuffer(vb);
    // m_CubeVA->SetIndexBuffer(ib);

    // ---------- Simple shader ----------
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
    // m_CubeVA.reset();
    m_Shader.reset();
}

void EditorLayer::DrawThemePanel()
{
    if (!m_ShowThemePanel)
        return;

    bool open = true;
    if (!ImGui::Begin("Theme Settings", &open))
    {
        ImGui::End();
        if (!open) m_ShowThemePanel = false;
        return;
    }

    if (!open)
    {
        m_ShowThemePanel = false;
        ImGui::End();
        return;
    }

    ImGuiStyle& s = ImGui::GetStyle();

    // ---------------- TOGGLE DEFAULT THEME ----------------
    bool useDefault = ThemeSettings::UseDefaultTheme;
    if (ImGui::Checkbox("Use Default Theme", &useDefault))
    {
        ThemeSettings::UseDefaultTheme = useDefault;

        if (useDefault)
        {
            // Restore engine built-in style (not imgui default)
            s = ImGuiLayer::DefaultEngineStyle;
        }
        else
        {
            ThemeSettings::ApplyThemeFromJSON();
        }
    }

    ImGui::Separator();
    ImGui::Spacing();

    // ---------------- Rounding ----------------
    ImGui::SliderFloat("Window Rounding", &s.WindowRounding, 0.f, 20.f);
    ImGui::SliderFloat("Frame Rounding",  &s.FrameRounding,  0.f, 20.f);
    ImGui::SliderFloat("Tab Rounding",    &s.TabRounding,    0.f, 20.f);

    ImGui::Spacing(); ImGui::Separator();

    // ---------------- Padding ----------------
    ImGui::SliderFloat2("Window Padding", (float*)&s.WindowPadding, 0.f, 30.f);
    ImGui::SliderFloat2("Frame Padding",  (float*)&s.FramePadding,  0.f, 30.f);
    ImGui::SliderFloat2("Item Spacing",   (float*)&s.ItemSpacing,   0.f, 30.f);

    ImGui::Spacing(); ImGui::Separator();

    // ---------------- Colors ----------------
    ImGui::Text("Colors");
    for (int i = 0; i < ImGuiCol_COUNT; i++)
    {
        const char* name = ImGui::GetStyleColorName(i);
        ImGui::ColorEdit4(name, (float*)&s.Colors[i]);
    }

    ImGui::Spacing(); ImGui::Separator();

    // ---------------- SAVE ----------------
    if (ImGui::Button("Save Theme"))
    {
        ThemeSettings::SaveThemeToJSON();
    }

    ImGui::End();
}



void EditorLayer::OnImGuiRender()
{
    // overriding user changes while editing in themes panel.
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
    // ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

    DrawThemePanel();
    DrawHierarchyPanel();
    DrawInspectorPanel();
    DrawContentBrowserPanel();
    DrawViewportPanel();

    // ImGui::PopStyleVar(2);
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
        ImVec2 windowPos   = ImGui::GetWindowPos();
        ImVec2 cursorPos   = ImGui::GetCursorPos();     // position inside window
        ImVec2 globalImage = { windowPos.x + cursorPos.x,
                               windowPos.y + cursorPos.y };
        
        ViewportInput::SetViewportBounds(
            globalImage.x,
            globalImage.y,
            m_ViewportSize.x,
            m_ViewportSize.y
        );
        ImGui::Text("VP Bounds: X=%f Y=%f  W=%f H=%f",
            globalImage.x,
            globalImage.y,
            m_ViewportSize.x,
            m_ViewportSize.y
        );


        // if (ViewportInput::IsCameraActive())
        // {
        //     double dx, dy;
        //     ViewportInput::GetMouseDelta(dx, dy);
        //     m_EditorCamera.OnMouseMoved((float)dx, (float)dy);
        // }

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

        // Bind shader and set view-projection matrix
        m_Shader->Bind();
        m_Shader->SetMat4("u_ViewProj", m_EditorCamera.GetViewProjection());

        // Loop through all entities with TransformComponent and MeshComponent
        auto& reg = m_ActiveScene->Reg();
        reg.view<TransformComponent, MeshComponent>().each(
            [&](auto entity, TransformComponent& transform, MeshComponent& meshComp)
            {
                if (!meshComp.MeshHandle)
                    return;

                // Set model matrix for this entity
                glm::mat4 model = transform.GetMatrix();
                m_Shader->SetMat4("u_Model", model);

                // Bind vertex array and draw
                auto* va = meshComp.MeshHandle->GetVertexArray();
                va->Bind();

                glDrawElements(GL_TRIANGLES,
                    meshComp.MeshHandle->GetIndexCount(),
                    GL_UNSIGNED_INT,
                    nullptr);
            });

        m_Framebuffer->Unbind();
        // -------------------------------

        uint32_t textureID = m_Framebuffer->GetColorAttachment();
        ImGui::Image((void*)(intptr_t)textureID,
            viewportPanelSize,
            ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();
}
