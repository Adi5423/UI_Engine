# Theme System

Documentation for the UI Engine's runtime theme customization system, including the ThemeSettings API, JSON persistence format, and integration with the editor.

---

## Overview

The theme system provides runtime customization of the ImGui-based editor interface. Users can modify all visual aspects of the editor and persist their preferences to a JSON file.

**Key Features:**
- Edit all ImGui style properties in real-time
- Persist themes to JSON file
- Restore default engine theme at any time
- Automatic theme loading on startup

---

## System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                    Application Startup                       │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│ ImGuiLayer::OnAttach()                                      │
│   1. Create ImGui context                                   │
│   2. Apply custom dark theme to ImGui::GetStyle()           │
│   3. Store DefaultEngineStyle = ImGui::GetStyle()           │
│   4. Call ThemeSettings::Init()                             │
│   5. If JSON exists, call ThemeSettings::ApplyThemeFromJSON │
└─────────────────────────────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────┐
│ Runtime (Editor Theme Panel)                                 │
│   - User modifies style via UI controls                      │
│   - "Save Theme" → ThemeSettings::SaveThemeToJSON()          │
│   - "Use Default" → ImGui::GetStyle() = DefaultEngineStyle   │
└─────────────────────────────────────────────────────────────┘
```

---

## ThemeSettings Class

**Location:** `Editor/Core/ThemeSettings.hpp/cpp`

### API Reference

#### Init

```cpp
static void ThemeSettings::Init();
```

**Purpose:** Initialize the theme system and load settings from JSON.

**Called:** Once during `ImGuiLayer::OnAttach()` after default style is defined.

**Behavior:**
1. Creates `settings/theme/` directory if it does not exist
2. Checks for `settings/theme/params.json`
3. If file exists, parses JSON and stores in memory
4. Sets `UseDefaultTheme` flag based on file contents or file existence

---

#### ApplyThemeFromJSON

```cpp
static void ThemeSettings::ApplyThemeFromJSON();
```

**Purpose:** Apply loaded JSON theme settings to `ImGui::GetStyle()`.

**Behavior:**
- Does nothing if `UseDefaultTheme == true` or if no JSON was loaded
- Applies style values (rounding, padding, spacing)
- Applies all color definitions

**Supported Style Properties:**

| Property | Type | JSON Key |
|----------|------|----------|
| WindowRounding | float | `"WindowRounding"` |
| FrameRounding | float | `"FrameRounding"` |
| TabRounding | float | `"TabRounding"` |
| WindowPadding | ImVec2 | `"WindowPadding"` (array) |
| FramePadding | ImVec2 | `"FramePadding"` (array) |
| ItemSpacing | ImVec2 | `"ItemSpacing"` (array) |

---

#### SaveThemeToJSON

```cpp
static void ThemeSettings::SaveThemeToJSON();
```

**Purpose:** Serialize the current `ImGui::GetStyle()` to JSON file.

**Output File:** `settings/theme/params.json`

**Behavior:**
1. Creates directory structure if needed
2. Writes all style properties
3. Writes all color values (RGBA format)
4. Pretty-prints with 4-space indentation

---

#### UseDefaultTheme

```cpp
static bool ThemeSettings::UseDefaultTheme;
```

**Purpose:** Flag indicating whether to use the default engine theme.

**When `true`:**
- `ApplyThemeFromJSON()` does nothing
- Theme panel shows "Use Default Theme" as checked

**When `false`:**
- JSON theme is applied (if loaded)
- Theme panel shows custom settings

---

## JSON File Format

**Location:** `settings/theme/params.json`

### Complete Example

```json
{
    "UseDefaultTheme": false,
    "WindowRounding": 8.0,
    "FrameRounding": 8.0,
    "TabRounding": 3.0,
    "WindowPadding": [8.0, 8.0],
    "FramePadding": [6.0, 4.0],
    "ItemSpacing": [8.0, 6.0],
    "Colors": {
        "Text": [1.0, 1.0, 1.0, 1.0],
        "TextDisabled": [0.5, 0.5, 0.5, 1.0],
        "WindowBg": [0.11, 0.11, 0.12, 1.0],
        "ChildBg": [0.07, 0.07, 0.08, 1.0],
        "PopupBg": [0.07, 0.07, 0.08, 1.0],
        "Border": [0.0, 0.0, 0.0, 0.5],
        "FrameBg": [0.16, 0.16, 0.18, 1.0],
        "FrameBgHovered": [0.20, 0.45, 0.80, 1.0],
        "FrameBgActive": [0.26, 0.59, 0.98, 1.0],
        "TitleBg": [0.07, 0.07, 0.08, 1.0],
        "TitleBgActive": [0.11, 0.11, 0.12, 1.0],
        "Button": [0.16, 0.16, 0.18, 1.0],
        "ButtonHovered": [0.20, 0.45, 0.80, 1.0],
        "ButtonActive": [0.26, 0.59, 0.98, 1.0],
        "Header": [0.16, 0.16, 0.18, 1.0],
        "HeaderHovered": [0.20, 0.45, 0.80, 1.0],
        "HeaderActive": [0.26, 0.59, 0.98, 1.0],
        "Tab": [0.07, 0.07, 0.08, 1.0],
        "TabHovered": [0.20, 0.45, 0.80, 1.0],
        "TabActive": [0.11, 0.11, 0.12, 1.0],
        "CheckMark": [0.26, 0.59, 0.98, 1.0],
        "SliderGrab": [0.20, 0.45, 0.80, 1.0],
        "SliderGrabActive": [0.26, 0.59, 0.98, 1.0]
    }
}
```

### Color Format

Colors are stored as arrays with 3 or 4 elements:

- **RGB:** `[r, g, b]` - Alpha defaults to 1.0
- **RGBA:** `[r, g, b, a]` - Explicit alpha

All values are floating-point in the range `0.0` to `1.0`.

### Color Names

All ImGui color indices are supported. Color names match ImGui's internal naming and can be retrieved programmatically:

```cpp
for (int i = 0; i < ImGuiCol_COUNT; ++i) {
    const char* name = ImGui::GetStyleColorName(i);
    // e.g., "Text", "WindowBg", "Button", etc.
}
```

---

## Default Engine Theme

The engine defines a custom dark theme in `ImGuiLayer::OnAttach()`:

### Style Properties

| Property | Value |
|----------|-------|
| WindowRounding | 8.0 |
| FrameRounding | 8.0 |
| TabRounding | 3.0 |
| ScrollbarRounding | 3.0 |
| GrabRounding | 2.0 |
| WindowBorderSize | 1.0 |
| FrameBorderSize | 2.8 |
| WindowPadding | (8, 8) |
| FramePadding | (6, 4) |
| ItemSpacing | (8, 6) |

### Color Palette

| Color Purpose | RGB Values | Description |
|---------------|------------|-------------|
| Background | (0.11, 0.11, 0.12) | Main window background |
| Background Dark | (0.07, 0.07, 0.08) | Child windows, popups |
| Background Light | (0.16, 0.16, 0.18) | Frames, buttons |
| Accent | (0.26, 0.59, 0.98) | Active elements (blue) |
| Accent Soft | (0.20, 0.45, 0.80) | Hover states |

### Backup Mechanism

Before loading user themes, the default style is stored:

```cpp
// In ImGuiLayer::OnAttach()
ImGuiLayer::DefaultEngineStyle = ImGui::GetStyle();
```

This enables restoration with:

```cpp
ImGui::GetStyle() = ImGuiLayer::DefaultEngineStyle;
```

---

## Theme Panel UI

**Location:** `EditorLayer::DrawThemePanel()`

### Access

- Menu: **Settings > Theme**
- Toggles `m_ShowThemePanel` flag

### Controls

The panel provides real-time editing for:

**Style Values:**
- Window Rounding (slider)
- Frame Rounding (slider)
- Tab Rounding (slider)
- Window Padding (2D input)
- Frame Padding (2D input)
- Item Spacing (2D input)

**Color Editors:**
- All ImGui colors displayed as collapsible sections
- Color pickers with alpha channel support

**Actions:**
- **Save Theme** - Calls `ThemeSettings::SaveThemeToJSON()`
- **Use Default Theme** - Checkbox that:
  - When checked: Restores `DefaultEngineStyle`
  - When unchecked: Applies custom settings

---

## Integration Example

### Adding Theme Support to a New Panel

```cpp
// Access current style
ImGuiStyle& style = ImGui::GetStyle();

// Modify a property
style.WindowRounding = 10.0f;

// Save changes
ThemeSettings::SaveThemeToJSON();
```

### Restoring Default Theme Programmatically

```cpp
// Set flag
ThemeSettings::UseDefaultTheme = true;

// Apply default style
ImGui::GetStyle() = ImGuiLayer::DefaultEngineStyle;

// Optionally save preference
ThemeSettings::SaveThemeToJSON();
```

---

## Related Documentation

- [Architecture Overview](ARCHITECTURE.md) - Theme system in context
- [ImGuiLayer](ARCHITECTURE.md#imguilayer) - ImGui initialization
- [EditorLayer](ARCHITECTURE.md#editorlayer) - Panel implementations
