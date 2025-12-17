#pragma once

#include <string>
#include <cstdint>

namespace WindowLayout
{
    enum class LayoutPreset
    {
        None = 0,
        Default,
        Godot
    };

    // Window layout presets - professional game engine style
    class LayoutManager
    {
    public:
        static void ApplyLayout(LayoutPreset preset);
        static void RebuildLayout(LayoutPreset preset, uint32_t dockspaceID);
        static void SaveCurrentLayout();
        
    private:
        static const char* GetLayoutData(LayoutPreset preset);
    };
}
