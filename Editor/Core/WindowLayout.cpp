#include "WindowLayout.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <fstream>

namespace WindowLayout
{
    void LayoutManager::ApplyLayout(LayoutPreset preset)
    {
        // This is now handled by the EditorApplication via RebuildLayout
        // We can still save to disk if we want persistence
    }

    void LayoutManager::RebuildLayout(LayoutPreset preset, uint32_t dockspaceID)
    {
        if (preset == LayoutPreset::None) return;

        ImGuiViewport* viewport = ImGui::GetMainViewport();

        ImGui::DockBuilderRemoveNode(dockspaceID);
        ImGui::DockBuilderAddNode(dockspaceID, ImGuiDockNodeFlags_DockSpace);
        ImGui::DockBuilderSetNodeSize(dockspaceID, viewport->Size);

        ImGuiID dockMain = dockspaceID;
        ImGuiID dockLeft, dockRight, dockBottom;

        if (preset == LayoutPreset::Default)
        {
            dockLeft = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.15f, nullptr, &dockMain);
            dockRight = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.20f, nullptr, &dockMain);
            dockBottom = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.25f, nullptr, &dockMain);

            ImGui::DockBuilderDockWindow("Hierarchy", dockLeft);
            ImGui::DockBuilderDockWindow("Inspector", dockRight);
            ImGui::DockBuilderDockWindow("Content Browser", dockBottom);
            ImGui::DockBuilderDockWindow("Viewport", dockMain);
        }
        else if (preset == LayoutPreset::Godot)
        {
            // Godot style: Left column with Hierarchy and Inspector, Bottom Content Browser, Central Viewport
            dockLeft = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.20f, nullptr, &dockMain);
            dockBottom = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.25f, nullptr, &dockMain);
            
            // Split dockLeft for Hierarchy (top) and Inspector (bottom)
            ImGuiID dockLeftTop, dockLeftBottom;
            dockLeftTop = ImGui::DockBuilderSplitNode(dockLeft, ImGuiDir_Up, 0.45f, nullptr, &dockLeftBottom);

            ImGui::DockBuilderDockWindow("Hierarchy", dockLeftTop);
            ImGui::DockBuilderDockWindow("Inspector", dockLeftBottom);
            ImGui::DockBuilderDockWindow("Content Browser", dockBottom);
            ImGui::DockBuilderDockWindow("Viewport", dockMain);
        }

        ImGui::DockBuilderFinish(dockspaceID);

        // Save to disk for persistence
        ImGui::SaveIniSettingsToDisk("imgui.ini");
    }

    void LayoutManager::SaveCurrentLayout()
    {
        ImGui::SaveIniSettingsToDisk("imgui.ini");
    }
}
