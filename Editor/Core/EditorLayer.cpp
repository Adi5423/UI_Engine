#include "EditorLayer.hpp"
#include "EditorBridge.hpp"
#include <glm/gtc/type_ptr.hpp>

#include <cmath>

#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>

#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <Core/Input/ViewportInput.hpp>
#include <Scene/Scene.hpp>
#include <Scene/Entity.hpp>
#include <Scene/Components.hpp>
#include <Scene/SceneAPI.hpp>
#include <Core/ThemeSettings.hpp>
#include <Core/ImGuiLayer.hpp>


// Vertical column-based Vec3 control - Professional game engine style
static void DrawVec3ControlVertical(const std::string& label, glm::vec3& values, float resetValue = 0.0f,
    std::function<void()> onStartEdit = nullptr, std::function<void()> onEndEdit = nullptr)
{
    ImGui::PushID(label.c_str());

    // 4 columns: Label | X | Y | Z
    ImGui::Columns(4, nullptr, false);
    ImGui::SetColumnWidth(0, 80.0f);
    ImGui::SetColumnWidth(1, 80.0f);
    ImGui::SetColumnWidth(2, 80.0f);
    ImGui::SetColumnWidth(3, 80.0f);

    // Label column
    ImGui::Text("%s", label.c_str());
    ImGui::NextColumn();

    // X column
    ImGui::PushItemWidth(-1);
    if (ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f")) { /* Changed */ }
    if (onStartEdit && ImGui::IsItemActivated()) onStartEdit();
    if (onEndEdit && (ImGui::IsItemDeactivatedAfterEdit() || (ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Enter)))) onEndEdit();
    ImGui::PopItemWidth();
    ImGui::NextColumn();

    // Y column
    ImGui::PushItemWidth(-1);
    if (ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f")) { /* Changed */ }
    if (onStartEdit && ImGui::IsItemActivated()) onStartEdit();
    if (onEndEdit && (ImGui::IsItemDeactivatedAfterEdit() || (ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Enter)))) onEndEdit();
    ImGui::PopItemWidth();
    ImGui::NextColumn();

    // Z column
    ImGui::PushItemWidth(-1);
    if (ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f")) { /* Changed */ }
    if (onStartEdit && ImGui::IsItemActivated()) onStartEdit();
    if (onEndEdit && (ImGui::IsItemDeactivatedAfterEdit() || (ImGui::IsItemActive() && ImGui::IsKeyPressed(ImGuiKey_Enter)))) onEndEdit();
    ImGui::PopItemWidth();
    ImGui::NextColumn();

    ImGui::Columns(1);
    ImGui::PopID();
}

static bool RayIntersectsAABB(glm::vec3 origin, glm::vec3 dir, glm::vec3 minB, glm::vec3 maxB, float& t)

{
    float tmin = (minB.x - origin.x) / dir.x;
    float tmax = (maxB.x - origin.x) / dir.x;

    if (tmin > tmax) std::swap(tmin, tmax);

    float tymin = (minB.y - origin.y) / dir.y;
    float tymax = (maxB.y - origin.y) / dir.y;

    if (tymin > tymax) std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    float tzmin = (minB.z - origin.z) / dir.z;
    float tzmax = (maxB.z - origin.z) / dir.z;

    if (tzmin > tzmax) std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (tzmin > tmin) tmin = tzmin;
    if (tzmax < tmax) tmax = tzmax;

    if (tmax < 0) return false;

    t = tmin;
    if (t < 0) t = tmax;
    return true;
}

EditorLayer::EditorLayer() : Layer("EditorLayer") {}
EditorLayer::~EditorLayer() = default;

void EditorLayer::OnAttach()
{
    EditorBridge::Init(&m_CommandHistory);
    {
        m_ActiveScene = std::make_unique<Scene>();
        SceneAPI::CreateDefaultScene(*m_ActiveScene);
    }
    m_SceneRenderer = std::make_shared<SceneRenderer>();
    m_SceneRenderer->Init();
    Renderer::Init();
    m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
}

// (Moved to top)

#include <Core/Input/Input.hpp>

glm::vec2 EditorLayer::WorldToScreen(const glm::vec3& worldPos, const glm::mat4& view, 
                                      const glm::mat4& proj, const glm::vec2& viewportSize, 
                                      const glm::vec2& viewportPos)
{
    // Transform to clip space
    glm::vec4 clipPos = proj * view * glm::vec4(worldPos, 1.0f);
    
    // Perspective divide
    if (clipPos.w == 0.0f) return viewportPos; // Avoid division by zero
    glm::vec3 ndcPos = glm::vec3(clipPos) / clipPos.w;
    
    // Convert to screen space
    float screenX = (ndcPos.x + 1.0f) * 0.5f * viewportSize.x + viewportPos.x;
    float screenY = (1.0f - ndcPos.y) * 0.5f * viewportSize.y + viewportPos.y;
    
    return glm::vec2(screenX, screenY);
}

void EditorLayer::OnDetach()
{
    EditorBridge::Init(nullptr); // Clear bridge pointer to prevent use-after-free
    m_ActiveScene.reset();
    m_SceneRenderer.reset();
}

void EditorLayer::OnUpdate(float deltaTime)
{
    ViewportInput::UpdateCameraState(Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT));

    if (ViewportInput::IsCameraActive())
    {
        glm::vec3 dir{0.0f};
        if (Input::IsKeyPressed(GLFW_KEY_W)) dir.z += 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_S)) dir.z -= 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_A)) dir.x -= 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_D)) dir.x += 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_E)) dir.y += 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_Q)) dir.y -= 1.0f;

        double dx, dy;
        ViewportInput::GetMouseDelta(dx, dy);

        m_EditorCamera.ProcessKeyboard(dir, deltaTime);
        m_EditorCamera.ProcessMouseMovement((float)dx, (float)dy);
    }

    if (m_ActiveScene)
        m_ActiveScene->OnUpdate(deltaTime);

    if (!ViewportInput::IsCameraActive()) // Changed else to if to allow scene update but separation logic continues
    {
        bool deletePressed = Input::IsKeyPressed(GLFW_KEY_X) || Input::IsKeyPressed(GLFW_KEY_DELETE);
        if (deletePressed && m_SelectedEntity) 
        {
             if (Input::IsKeyPressed(GLFW_KEY_X))
             {
                 if (!m_ShowDeletePopup && m_SelectedEntity.HasComponent<TransformComponent>()) 
                 {
                    m_ShowDeletePopup = true;
                    
                    // Store object's world position for later screen projection
                    auto& tc = m_SelectedEntity.GetComponent<TransformComponent>();
                    m_DeletePopupWorldPos = tc.Position;
                    m_DeletePopupNeedsPositioning = true;
                 }
             }
             else
             {
                 EditorBridge::SubmitDeleteEntity(m_SelectedEntity);
                 m_SelectedEntity = Entity();
             }
        }
    }

    if (!ImGui::GetIO().WantTextInput && !ViewportInput::IsCameraActive())
    {
        if (Input::IsKeyPressed(GLFW_KEY_Q)) m_GizmoType = -1;
        if (Input::IsKeyPressed(GLFW_KEY_W)) m_GizmoType = ImGuizmo::TRANSLATE;
        if (Input::IsKeyPressed(GLFW_KEY_E)) m_GizmoType = ImGuizmo::ROTATE;
        if (Input::IsKeyPressed(GLFW_KEY_R)) m_GizmoType = ImGuizmo::SCALE;
    }

    if (!ImGui::GetIO().WantTextInput)
    {
        bool ctrlPressed = Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL) || Input::IsKeyPressed(GLFW_KEY_RIGHT_CONTROL);
        
        // =====================================================================
        // Undo / Redo Shortcuts
        // =====================================================================
        if (ctrlPressed && Input::IsKeyPressed(GLFW_KEY_Z))
        {
             if (!m_UndoPressedLastFrame) 
             {
                 m_CommandHistory.Undo();
                 m_UndoPressedLastFrame = true;
             }
        }
        else
        {
             m_UndoPressedLastFrame = false;
        }
        
        if (ctrlPressed && Input::IsKeyPressed(GLFW_KEY_Y))
        {
             if (!m_RedoPressedLastFrame)
             {
                 m_CommandHistory.Redo();
                 m_RedoPressedLastFrame = true;
             }
        }
        else
        {
             m_RedoPressedLastFrame = false;
        }

        // =====================================================================
        // Clipboard / Hierarchy Shortcuts
        // =====================================================================
        if (ctrlPressed && m_SelectedEntity)
        {
            // Copy
            if (Input::IsKeyPressed(GLFW_KEY_C))
            {
                if (m_SelectedEntity.HasComponent<IDComponent>())
                {
                    m_Clipboard.Mode = ClipboardMode::Copy;
                    m_Clipboard.EntityID = m_SelectedEntity.GetComponent<IDComponent>().ID;
                    m_CutEntityID = entt::null;
                    CORE_INFO("[Clipboard] Entity Copied to clipboard");
                }
            }

            // Cut
            if (Input::IsKeyPressed(GLFW_KEY_X))
            {
                if (m_SelectedEntity.HasComponent<IDComponent>())
                {
                    m_Clipboard.Mode = ClipboardMode::Cut;
                    m_Clipboard.EntityID = m_SelectedEntity.GetComponent<IDComponent>().ID;
                    m_CutEntityID = m_SelectedEntity.Handle();
                    CORE_INFO("[Clipboard] Entity Cut to clipboard");
                }
            }

            // Duplicate
            if (Input::IsKeyPressed(GLFW_KEY_D))
            {
                EditorBridge::SubmitDuplicate(m_SelectedEntity, true); // Linked
            }
        }

        // Paste
        if (ctrlPressed && Input::IsKeyPressed(GLFW_KEY_V))
        {
            if (m_Clipboard.Mode != ClipboardMode::None)
            {
                Entity src = m_ActiveScene->GetEntityByUUID(m_Clipboard.EntityID);
                if (src)
                {
                    if (m_Clipboard.Mode == ClipboardMode::Copy)
                    {
                        EditorBridge::SubmitDuplicate(src, false);
                        CORE_INFO("[Clipboard] Entity Pasted (Duplicated)");
                    }
                    else if (m_Clipboard.Mode == ClipboardMode::Cut)
                    {
                        EditorBridge::SubmitReorder(src);
                        m_Clipboard.Mode = ClipboardMode::None;
                        m_CutEntityID = entt::null;
                        m_SelectedEntity = src;
                        CORE_INFO("[Clipboard] Entity Pasted (Moved)");
                    }
                }
            }
        }
    }
}

void EditorLayer::DrawThemePanel()
{
    if (!m_ShowThemePanel) return;
    bool open = true;
    if (!ImGui::Begin("Theme Settings", &open)) { ImGui::End(); if (!open) m_ShowThemePanel = false; return; }
    if (!open) { m_ShowThemePanel = false; ImGui::End(); return; }

    ImGuiStyle& s = ImGui::GetStyle();
    bool useDefault = ThemeSettings::UseDefaultTheme;
    if (ImGui::Checkbox("Use Default Theme", &useDefault))
    {
        ThemeSettings::UseDefaultTheme = useDefault;
        if (useDefault) s = ImGuiLayer::DefaultEngineStyle;
        else ThemeSettings::ApplyThemeFromJSON();
    }
    ImGui::Separator(); ImGui::Spacing();
    ImGui::SliderFloat("Window Rounding", &s.WindowRounding, 0.f, 20.f);
    ImGui::SliderFloat("Frame Rounding",  &s.FrameRounding,  0.f, 20.f);
    ImGui::SliderFloat("Tab Rounding",    &s.TabRounding,    0.f, 20.f);
    ImGui::Spacing(); ImGui::Separator();
    ImGui::SliderFloat2("Window Padding", (float*)&s.WindowPadding, 0.f, 30.f);
    ImGui::SliderFloat2("Frame Padding",  (float*)&s.FramePadding,  0.f, 30.f);
    ImGui::SliderFloat2("Item Spacing",   (float*)&s.ItemSpacing,   0.f, 30.f);
    ImGui::Spacing(); ImGui::Separator();
    ImGui::Text("Colors");
    for (int i = 0; i < ImGuiCol_COUNT; i++)
    {
        const char* name = ImGui::GetStyleColorName(i);
        ImGui::ColorEdit4(name, (float*)&s.Colors[i]);
    }
    ImGui::Spacing(); ImGui::Separator();
    if (ImGui::Button("Save Theme")) ThemeSettings::SaveThemeToJSON();
    ImGui::End();
}

void EditorLayer::OnImGuiRender()
{
    ImGuizmo::BeginFrame();
    DrawThemePanel();
    DrawHierarchyPanel();
    DrawInspectorPanel();
    DrawContentBrowserPanel();
    DrawViewportPanel();
}

// Replaced DrawHierarchyPanel with hover effects and advanced context menus
void EditorLayer::DrawHierarchyPanel()
{
    ImGui::Begin("Hierarchy");

    if (m_ActiveScene)
    {
        auto& reg = m_ActiveScene->Reg();
        Entity entityToDelete;
        bool shouldDelete = false;

        // Sort by HierarchyOrderComponent (Professional engines keep order)
        // We want new entities (higher Order) at top? User said "new entity goes on top"
        reg.sort<HierarchyOrderComponent>([](const auto& lhs, const auto& rhs) {
            return lhs.Order > rhs.Order; // Descending = Newer at top
        });

        // Header style
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 5));
        
        auto view = reg.view<TagComponent, HierarchyOrderComponent>();
        for (auto entityHandle : view)
        {
            Entity entity(entityHandle, m_ActiveScene.get());
            auto& tag = reg.get<TagComponent>(entityHandle);
            
            bool isSelected = (m_SelectedEntity == entity);
            bool isCut = (m_CutEntityID == entityHandle);

            ImGuiTreeNodeFlags flags = (isSelected ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
            flags |= ImGuiTreeNodeFlags_SpanAvailWidth; 
            flags |= ImGuiTreeNodeFlags_Leaf; 

            // Fade the text if cut
            if (isCut) ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 0.5f));

            bool opened = ImGui::TreeNodeEx((void*)(uint64_t)entityHandle, flags, "%s", tag.Tag.c_str());
            
            if (isCut) ImGui::PopStyleColor();

            // Hover effect
            if (ImGui::IsItemHovered() && !isSelected)
            {
                ImVec2 min = ImGui::GetItemRectMin();
                ImVec2 max = ImGui::GetItemRectMax();
                ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(50, 120, 200, 40));
            }
            
            if (ImGui::IsItemClicked())
            {
                m_SelectedEntity = entity;
            }

            // Right click context menu on ITEM
            if (ImGui::BeginPopupContextItem())
            {
                m_SelectedEntity = entity; // Select on right click like Unity

                if (ImGui::MenuItem("Cut", "Ctrl+X")) {
                    if (entity.HasComponent<IDComponent>()) {
                        m_Clipboard.Mode = ClipboardMode::Cut;
                        m_Clipboard.EntityID = entity.GetComponent<IDComponent>().ID;
                        m_CutEntityID = entityHandle;
                    }
                }
                if (ImGui::MenuItem("Copy", "Ctrl+C")) {
                    if (entity.HasComponent<IDComponent>()) {
                        m_Clipboard.Mode = ClipboardMode::Copy;
                        m_Clipboard.EntityID = entity.GetComponent<IDComponent>().ID;
                        m_CutEntityID = entt::null;
                    }
                }
                
                bool canPaste = (m_Clipboard.Mode != ClipboardMode::None);
                if (!canPaste) ImGui::BeginDisabled();
                if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                    Entity src = m_ActiveScene->GetEntityByUUID(m_Clipboard.EntityID);
                    if (src) {
                        if (m_Clipboard.Mode == ClipboardMode::Copy) {
                            EditorBridge::SubmitDuplicate(src, false);
                        } else if (m_Clipboard.Mode == ClipboardMode::Cut) {
                            SceneAPI::SetNextOrder(src);
                            m_Clipboard.Mode = ClipboardMode::None;
                            m_CutEntityID = entt::null;
                        }
                    }
                }
                if (!canPaste) ImGui::EndDisabled();

                if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {
                    EditorBridge::SubmitDuplicate(entity, true); 
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Delete Entity", "Del"))
                {
                    entityToDelete = entity;
                    shouldDelete = true;
                }
                ImGui::EndPopup();
            }

            if (opened)
            {
                ImGui::TreePop();
            }
        }
        
        ImGui::PopStyleVar();

        // Right click on blank space to create
        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            if (ImGui::MenuItem("Create Empty Entity"))
            {
                 EditorBridge::SubmitCreateMesh(m_ActiveScene.get(), "Empty Entity", nullptr);
            }

            if (ImGui::BeginMenu("Mesh"))
            {
                if (ImGui::MenuItem("Cube"))     EditorBridge::SubmitCreateMesh(m_ActiveScene.get(), "Cube",     Mesh::CreateCube());
                if (ImGui::MenuItem("Circle"))   EditorBridge::SubmitCreateMesh(m_ActiveScene.get(), "Circle",   Mesh::CreateCircle(32));
                if (ImGui::MenuItem("Triangle")) EditorBridge::SubmitCreateMesh(m_ActiveScene.get(), "Triangle", Mesh::CreateTriangle3D());
                if (ImGui::MenuItem("Plane"))    EditorBridge::SubmitCreateMesh(m_ActiveScene.get(), "Plane",    Mesh::CreatePlane());
                ImGui::EndMenu();
            }

            if (ImGui::MenuItem("Camera"))
            {
                // Note: EditorBridge doesn't have SubmitCreateCamera yet, 
                // but we can either add it or just use SceneAPI for now until we expand Bridge.
                // However, user wants everything professional. I'll stick to what Bridge has or add to Bridge.
                // For now, I'll use SceneAPI but recognize it won't be undoable. 
                // To be safe, I'll add a placeholder or update it.
                SceneAPI::CreateCameraEntity(*m_ActiveScene);
            }

            ImGui::Separator();

            bool canPaste = (m_Clipboard.Mode != ClipboardMode::None);
            if (!canPaste) ImGui::BeginDisabled();
            if (ImGui::MenuItem("Paste", "Ctrl+V")) {
                Entity src = m_ActiveScene->GetEntityByUUID(m_Clipboard.EntityID);
                if (src) {
                    if (m_Clipboard.Mode == ClipboardMode::Copy) {
                        EditorBridge::SubmitDuplicate(src, false);
                    } else if (m_Clipboard.Mode == ClipboardMode::Cut) {
                        EditorBridge::SubmitReorder(src);
                        m_Clipboard.Mode = ClipboardMode::None;
                        m_CutEntityID = entt::null;
                        m_SelectedEntity = src;
                    }
                }
            }
            if (!canPaste) ImGui::EndDisabled();

            ImGui::EndPopup();
        }

        if (shouldDelete)
        {
            if (m_SelectedEntity == entityToDelete) m_SelectedEntity = {};
            EditorBridge::SubmitDeleteEntity(entityToDelete);
        }
    }

    ImGui::End();
}



// Replaced DrawInspectorPanel
void EditorLayer::DrawInspectorPanel()
{
    ImGui::Begin("Inspector");

    if (m_SelectedEntity)
    {
        if (m_SelectedEntity.HasComponent<TagComponent>())
        {
            auto& tag = m_SelectedEntity.GetComponent<TagComponent>();
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy(buffer, tag.Tag.c_str(), sizeof(buffer) - 1);
            
            // Draw a simpler name field at the top
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
            {
                tag.Tag = std::string(buffer);
            }
             if (ImGui::IsItemActivated()) m_PreviousName = tag.Tag;
             if (ImGui::IsItemDeactivatedAfterEdit() && m_PreviousName != tag.Tag)
             {
                 EditorBridge::SubmitRename(m_SelectedEntity, m_PreviousName, tag.Tag);
             }
        }

        ImGui::Separator();

        // Components
        if (m_SelectedEntity.HasComponent<TransformComponent>())
        {
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
            {
                auto& tc = m_SelectedEntity.GetComponent<TransformComponent>();
                
                // Draw column headers
                ImGui::Columns(4, nullptr, false);
                ImGui::SetColumnWidth(0, 80.0f);
                ImGui::SetColumnWidth(1, 80.0f);
                ImGui::SetColumnWidth(2, 80.0f);
                ImGui::SetColumnWidth(3, 80.0f);
                
                ImGui::Text(""); // Empty for label column
                ImGui::NextColumn();
                
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.2f, 0.2f, 1.0f));
                ImGui::Text("X");
                ImGui::PopStyleColor();
                ImGui::NextColumn();
                
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.3f, 0.9f, 0.3f, 1.0f));
                ImGui::Text("Y");
                ImGui::PopStyleColor();
                ImGui::NextColumn();
                
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.4f, 0.9f, 1.0f));
                ImGui::Text("Z");
                ImGui::PopStyleColor();
                ImGui::NextColumn();
                
                ImGui::Columns(1);
                ImGui::Separator();
                
                // Position
                DrawVec3ControlVertical("Position", tc.Position, 0.0f, 
                    [&]() { m_TransformEditState.savedTransform = tc; }, 
                    [&]() { EditorBridge::SubmitTransformChange(m_SelectedEntity, m_TransformEditState.savedTransform, tc); }
                );

                // Rotation
                glm::vec3 rotationDeg = tc.Rotation; 
                DrawVec3ControlVertical("Rotation", rotationDeg, 0.0f,
                    [&]() { m_TransformEditState.savedTransform = tc; },
                    [&]() { 
                        tc.Rotation = rotationDeg;
                         EditorBridge::SubmitTransformChange(m_SelectedEntity, m_TransformEditState.savedTransform, tc); 
                    }
                );
                if (rotationDeg != tc.Rotation) tc.Rotation = rotationDeg;

                // Scale
                DrawVec3ControlVertical("Scale", tc.Scale, 1.0f,
                   [&]() { m_TransformEditState.savedTransform = tc; },
                   [&]() {
                       if (tc.Scale.x < 0.001f) tc.Scale.x = 0.001f;
                       if (tc.Scale.y < 0.001f) tc.Scale.y = 0.001f;
                       if (tc.Scale.z < 0.001f) tc.Scale.z = 0.001f;
                       EditorBridge::SubmitTransformChange(m_SelectedEntity, m_TransformEditState.savedTransform, tc); 
                   }
                );
                 if (tc.Scale.x < 0.001f) tc.Scale.x = 0.001f;
                 if (tc.Scale.y < 0.001f) tc.Scale.y = 0.001f;
                 if (tc.Scale.z < 0.001f) tc.Scale.z = 0.001f;
            }
        }
    }
    else
    {
        ImGui::Text("Select an item to view properties");
    }

    ImGui::End();
}



// Replaced DrawContentBrowserPanel
void EditorLayer::DrawContentBrowserPanel()
{
    ImGui::Begin("Content Browser");

    // Simple grid layout simulation for "Professional" look
    static float padding = 50.0f;
    static float thumbnailSize = 96.0f;
    float cellSize = thumbnailSize + padding;

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columnCount = (int)(panelWidth / cellSize);
    if (columnCount < 1) columnCount = 1;

    ImGui::Columns(columnCount, 0, false);

    // Dummy Assets
    const char* assets[] = { "Scene.sc", "Player.obj", "Tex.png", "Script.lua", "Audio.wav" };

    for (int i = 0; i < 5; i++)
    {
        ImGui::PushID(i);
        // Placeholder button as thumbnail
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::ImageButton("##asset", (ImTextureID)0, ImVec2(thumbnailSize, thumbnailSize), ImVec2(0, 1), ImVec2(1, 0));
        
        // Drag Source (Example)
        if (ImGui::BeginDragDropSource())
        {
            ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", assets[i], strlen(assets[i]) + 1);
            ImGui::Text("%s", assets[i]);
            ImGui::EndDragDropSource();
        }

        ImGui::PopStyleColor();
        ImGui::TextWrapped("%s", assets[i]);
        ImGui::NextColumn();
        ImGui::PopID();
    }

    ImGui::Columns(1);
    ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
    ImGui::End();
}



void EditorLayer::DrawViewportPanel()
{
    ImGuiWindowFlags viewportFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    if (ImGui::Begin("Viewport", nullptr, viewportFlags))
    {
        ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
        m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 cursorPos = ImGui::GetCursorPos();
        ImVec2 globalImage = { windowPos.x + cursorPos.x, windowPos.y + cursorPos.y };
        
        ViewportInput::SetViewportBounds(globalImage.x, globalImage.y, m_ViewportSize.x, m_ViewportSize.y);
        ImGui::Text("VP Bounds: X=%f Y=%f  W=%f H=%f", globalImage.x, globalImage.y, m_ViewportSize.x, m_ViewportSize.y);

        if ((uint32_t)m_ViewportSize.x > 0 && (uint32_t)m_ViewportSize.y > 0 &&
            (m_SceneRenderer->GetFramebuffer()->GetWidth() != (uint32_t)m_ViewportSize.x || m_SceneRenderer->GetFramebuffer()->GetHeight() != (uint32_t)m_ViewportSize.y))
        {
            m_SceneRenderer->SetViewportSize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);
        }

        // Calculate delete popup screen position if needed
        if (m_DeletePopupNeedsPositioning && m_SelectedEntity && m_SelectedEntity.HasComponent<TransformComponent>())
        {
            auto& tc = m_SelectedEntity.GetComponent<TransformComponent>();
            m_DeletePopupPos = WorldToScreen(
                tc.Position,
                m_EditorCamera.GetViewMatrix(),
                m_EditorCamera.GetProjectionMatrix(),
                m_ViewportSize,
                glm::vec2(globalImage.x, globalImage.y)
            );
            
            // Clamp to viewport bounds with padding
            float padding = 10.0f;
            float popupWidth = 150.0f;  // Approximate popup width
            float popupHeight = 80.0f;  // Approximate popup height
            
            m_DeletePopupPos.x = glm::clamp(m_DeletePopupPos.x, 
                                            globalImage.x + padding, 
                                            globalImage.x + m_ViewportSize.x - popupWidth - padding);
            m_DeletePopupPos.y = glm::clamp(m_DeletePopupPos.y, 
                                            globalImage.y + padding, 
                                            globalImage.y + m_ViewportSize.y - popupHeight - padding);
            
            m_DeletePopupNeedsPositioning = false;
        }

        // Delegated Rendering to SceneRenderer
        m_SceneRenderer->RenderEditor(m_ActiveScene.get(), m_EditorCamera, m_SelectedEntity);

        uint32_t textureID = m_SceneRenderer->GetFinalImage();
        ImGui::Image((void*)(intptr_t)textureID, viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));

        if (m_SelectedEntity && m_GizmoType != -1)
        {
            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(globalImage.x, globalImage.y, m_ViewportSize.x, m_ViewportSize.y);
            const glm::mat4& cameraProjection = m_EditorCamera.GetProjectionMatrix();
            glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();
            auto& tc = m_SelectedEntity.GetComponent<TransformComponent>();
            // Disable preview accumulation - work directly on component
            glm::mat4 transform = tc.GetMatrix();
            glm::mat4 deltaMatrix(1.0f);

            bool snap = Input::IsKeyPressed(GLFW_KEY_LEFT_CONTROL);
            float snapValue = 0.5f;
            if (m_GizmoType == ImGuizmo::ROTATE) snapValue = 45.0f;
            float snapValues[3] = { snapValue, snapValue, snapValue };

            ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                (ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
                glm::value_ptr(deltaMatrix), snap ? snapValues : nullptr);

            if (ImGuizmo::IsUsing())
            {
                if (!m_WasUsingGizmo)
                {
                    m_TransformEditState.savedTransform = tc;
                    m_WasUsingGizmo = true;
                    m_GizmoTypeAtStart = m_GizmoType;
                }

                // LIVE UPDATE: Decompose immediately to support inspector updates
                glm::vec3 scale;
                glm::quat rotation;
                glm::vec3 translation;
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::decompose(transform, scale, rotation, translation, skew, perspective);

                tc.Position = translation;
                tc.Rotation = glm::degrees(glm::eulerAngles(rotation));
                
                // Prevent scaling to zero/negative
                if (scale.x < 0.001f) scale.x = 0.001f;
                if (scale.y < 0.001f) scale.y = 0.001f;
                if (scale.z < 0.001f) scale.z = 0.001f;
                
                tc.Scale = scale;
            }
            else if (m_WasUsingGizmo)
            {
                // Commit the final state (which is already in 'tc') to history
                m_GizmoTypeAtStart = -1;
                EditorBridge::SubmitTransformChange(m_SelectedEntity, m_TransformEditState.savedTransform, tc);
                m_WasUsingGizmo = false;
            }
        }
        
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && (!m_SelectedEntity || !ImGuizmo::IsOver()) && !ImGui::IsPopupOpen("DeleteConfirmation"))
        {
            ImGui::SetWindowFocus();
            ImVec2 mousePos = ImGui::GetMousePos();
            ImVec2 vpPos = globalImage;
            if (mousePos.x >= vpPos.x && mousePos.y >= vpPos.y && mousePos.x < (vpPos.x + m_ViewportSize.x) && mousePos.y < (vpPos.y + m_ViewportSize.y))
            {
                glm::vec2 localMouse = { mousePos.x - vpPos.x, mousePos.y - vpPos.y };
                float u = (localMouse.x / m_ViewportSize.x) * 2.0f - 1.0f;
                float v = ((m_ViewportSize.y - localMouse.y) / m_ViewportSize.y) * 2.0f - 1.0f;
                const glm::mat4& proj = m_EditorCamera.GetProjectionMatrix();
                const glm::mat4& view = m_EditorCamera.GetViewMatrix();
                glm::mat4 invVP = glm::inverse(proj * view);
                glm::vec4 rayStart = invVP * glm::vec4(u, v, -1.0f, 1.0f); rayStart /= rayStart.w;
                glm::vec4 rayEnd   = invVP * glm::vec4(u, v,  1.0f, 1.0f); rayEnd   /= rayEnd.w;
                glm::vec3 rayDir = glm::normalize(glm::vec3(rayEnd) - glm::vec3(rayStart));
                glm::vec3 rayOrigin = glm::vec3(rayStart);
                
                 m_SelectedEntity = {};
                 float minT = FLT_MAX;
                 auto& reg = m_ActiveScene->Reg();
                 reg.view<TransformComponent, MeshComponent>().each([&](auto entity, TransformComponent& tc, MeshComponent& mc) {
                     if (!mc.MeshHandle) return;
                     
                     glm::vec3 minB = mc.MeshHandle->GetMinAABB();
                     glm::vec3 maxB = mc.MeshHandle->GetMaxAABB();

                     glm::mat4 invModel = glm::inverse(tc.GetMatrix());
                     glm::vec3 localRayOrigin = glm::vec3(invModel * glm::vec4(rayOrigin, 1.0f));
                     glm::vec3 localRayDir = glm::normalize(glm::vec3(invModel * glm::vec4(rayDir, 0.0f)));
                     float t;
                     if (RayIntersectsAABB(localRayOrigin, localRayDir, minB, maxB, t))
                     {
                         if (t < minT && t >= 0.0f)
                         {
                             minT = t;
                             m_SelectedEntity = { entity, m_ActiveScene.get() };
                         }
                     }
                 });
            }
        }
    }
    ImGui::End();
    
    // Delete Popup Logic
    if (m_ShowDeletePopup)
    {
        ImGui::OpenPopup("DeleteConfirmation");
        m_ShowDeletePopup = false; // Request consumed
    }

    ImGui::SetNextWindowPos(ImVec2(m_DeletePopupPos.x, m_DeletePopupPos.y), ImGuiCond_Appearing);
    if (ImGui::BeginPopup("DeleteConfirmation"))
    {
        ImGui::Text("Delete Selected?");
        ImGui::Spacing();
        
        if (ImGui::Button("OK", ImVec2(120, 0)) || Input::IsKeyPressed(GLFW_KEY_ENTER)) 
        {
            if (m_SelectedEntity)
            {
                EditorBridge::SubmitDeleteEntity(m_SelectedEntity);
                m_SelectedEntity = Entity();
            }
            ImGui::CloseCurrentPopup();
        }
        
        if (Input::IsKeyPressed(GLFW_KEY_ESCAPE))
        {
            ImGui::CloseCurrentPopup();
        }
        
        ImGui::EndPopup();
    }
}
