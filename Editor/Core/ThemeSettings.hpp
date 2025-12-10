#pragma once
#include <imgui.h>
#include <string>
#include <glm/glm.hpp>

class ThemeSettings
{
public:
    static void Init();                    // load at engine startup
    static void ApplyThemeFromJSON();      // apply JSON overrides
    static void SaveThemeToJSON();         // write current style into params.json
    
    static bool UseDefaultTheme;           // controlled by UI tab

private:
    static std::string GetThemeFilePath();
    static void LoadJSON();
    static void EnsureDirs();
};
