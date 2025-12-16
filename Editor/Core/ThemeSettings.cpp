// Engine/Core/ThemeSettings.cpp
#include "ThemeSettings.hpp"
#include <imgui.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <json/json.hpp> // vendor/json/json.hpp
#include <Core/Log.hpp>

using json = nlohmann::json;

bool ThemeSettings::UseDefaultTheme = false;
static json g_ThemeJSON; // loaded json

static std::string GetThemeFilePathInternal() { return "settings/theme/params.json"; }

void ThemeSettings::EnsureDirs()
{
    std::filesystem::create_directories("settings/theme");
}

void ThemeSettings::Init()
{
    EnsureDirs();

    std::string path = GetThemeFilePathInternal();

    if (!std::filesystem::exists(path))
    {
        CORE_WARN("[Theme] No theme file found. Using default theme.");
        UseDefaultTheme = true;
        return;
    }

    std::ifstream f(path);
    if (!f.is_open())
    {
        CORE_WARN("[Theme] Failed open theme file. Using default theme.");
        UseDefaultTheme = true;
        return;
    }

    try
    {
        f >> g_ThemeJSON;
    }
    catch (const std::exception& e)
    {
        CORE_ERROR("[Theme] JSON parse error: {0}", e.what());
        UseDefaultTheme = true;
        f.close();
        return;
    }
    f.close();

    if (g_ThemeJSON.contains("UseDefaultTheme"))
        UseDefaultTheme = g_ThemeJSON["UseDefaultTheme"].get<bool>();
}

void ThemeSettings::ApplyThemeFromJSON()
{
    if (UseDefaultTheme || g_ThemeJSON.empty())
        return;

    ImGuiStyle& style = ImGui::GetStyle();

    // Basic floats (safely check existence)
    if (g_ThemeJSON.contains("WindowRounding")) style.WindowRounding = g_ThemeJSON["WindowRounding"].get<float>();
    if (g_ThemeJSON.contains("FrameRounding"))  style.FrameRounding  = g_ThemeJSON["FrameRounding"].get<float>();
    if (g_ThemeJSON.contains("TabRounding"))    style.TabRounding    = g_ThemeJSON["TabRounding"].get<float>();

    if (g_ThemeJSON.contains("WindowPadding") && g_ThemeJSON["WindowPadding"].is_array() && g_ThemeJSON["WindowPadding"].size() >= 2)
    {
        style.WindowPadding = ImVec2(g_ThemeJSON["WindowPadding"][0].get<float>(), g_ThemeJSON["WindowPadding"][1].get<float>());
    }
    if (g_ThemeJSON.contains("FramePadding") && g_ThemeJSON["FramePadding"].is_array() && g_ThemeJSON["FramePadding"].size() >= 2)
    {
        style.FramePadding = ImVec2(g_ThemeJSON["FramePadding"][0].get<float>(), g_ThemeJSON["FramePadding"][1].get<float>());
    }
    if (g_ThemeJSON.contains("ItemSpacing") && g_ThemeJSON["ItemSpacing"].is_array() && g_ThemeJSON["ItemSpacing"].size() >= 2)
    {
        style.ItemSpacing = ImVec2(g_ThemeJSON["ItemSpacing"][0].get<float>(), g_ThemeJSON["ItemSpacing"][1].get<float>());
    }

    // Colors
    if (!g_ThemeJSON.contains("Colors"))
        return;

    json& colorsJson = g_ThemeJSON["Colors"];

    // For each ImGui color index, find JSON entry by comparing ImGui style color names.
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
    {
        const char* name = ImGui::GetStyleColorName(i);
        if (!name) continue;

        std::string key(name);
        if (colorsJson.contains(key))
        {
            auto& arr = colorsJson[key];
            // accept 3 or 4 elements (rgb or rgba)
            if (arr.is_array() && (arr.size() == 3 || arr.size() == 4))
            {
                float r = arr[0].get<float>();
                float g = arr[1].get<float>();
                float b = arr[2].get<float>();
                float a = (arr.size() == 4) ? arr[3].get<float>() : 1.0f;
                style.Colors[i] = ImVec4(r, g, b, a);
            }
        }
    }
}

void ThemeSettings::SaveThemeToJSON()
{
    ImGuiStyle& s = ImGui::GetStyle();

    json j;
    j["UseDefaultTheme"] = UseDefaultTheme;

    j["WindowRounding"] = s.WindowRounding;
    j["FrameRounding"]  = s.FrameRounding;
    j["TabRounding"]    = s.TabRounding;

    j["WindowPadding"] = { s.WindowPadding.x, s.WindowPadding.y };
    j["FramePadding"]  = { s.FramePadding.x,  s.FramePadding.y };
    j["ItemSpacing"]   = { s.ItemSpacing.x,   s.ItemSpacing.y };

    json colorsJson;
    for (int i = 0; i < ImGuiCol_COUNT; ++i)
    {
        const char* name = ImGui::GetStyleColorName(i);
        if (!name) continue;
        auto& c = s.Colors[i];
        // store RGBA
        colorsJson[std::string(name)] = { c.x, c.y, c.z, c.w };
    }
    j["Colors"] = colorsJson;

    EnsureDirs();
    std::string path = GetThemeFilePathInternal();
    std::ofstream out(path);
    if (!out.is_open())
    {
        CORE_ERROR("[Theme] Failed to open file for saving: {0}", path);
        return;
    }
    out << j.dump(4);
    out.close();

    CORE_INFO("[Theme] Theme saved to {0}", path);
}
