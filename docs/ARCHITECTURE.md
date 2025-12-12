# UI Engine Architecture

This document provides a technical overview of the UI Engine's architecture, including the core systems, their responsibilities, and how they interact.

---

## System Overview

The UI Engine follows a modular, professional game engine architecture with clear separation between:
- **Engine Module**: Core runtime library providing application framework, rendering, and scene management
- **Editor Module**: ImGui-based editor application built on top of the engine

```
┌─────────────────────────────────────────────┐
│          Editor Entry Point                 │
│      (Defines CreateApplication)            │
└─────────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────┐
│          Editor Application                 │
│      (Inherits from Application)            │
│  ┌──────────────┐  ┌──────────────────┐    │
│  │ ImGuiLayer   │  │  EditorLayer     │    │
│  │              │  │  - Viewport      │    │
│  │              │  │  - Scene Panel   │    │
│  └──────────────┘  └──────────────────┘    │
└─────────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────┐
│           Engine Library                     │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  │
│  │   Core   │  │ Rendering│  │  Scene   │  │
│  │(EntryPoint) │          │  │  (ECS)   │  │
│  └──────────┘  └──────────┘  └──────────┘  │
└─────────────────────────────────────────────┘
                    │
                    ▼
┌─────────────────────────────────────────────┐
│       Vendor Libraries                       │
│  GLFW │ GLAD │ GLM │ ImGui │ EnTT │ STB    │
└─────────────────────────────────────────────┘
```

---

## Engine Module

The engine is compiled as a static library (`UICheckEngine`) that provides all core functionality.

### Core Subsystem

**Location:** `Engine/Core/`

#### Entry Point (`EntryPoint.hpp`)
- Contains the engine's `main()` function
- Handles bootstrap, argument parsing, and shutdown
- Calls the client-defined `CreateApplication()` function

#### Application (`Application.hpp/cpp`)
- **Base Class** for all applications
- Virtual lifecycle methods: `OnInit()`, `OnUpdate(dt)`, `OnShutdown()`
- Manages the main run loop and window
- Singleton access via `Application::Get()`

#### Window (`Window.hpp`, `GLFWWindow.hpp/cpp`)
- Platform-agnostic window interface
- GLFW implementation for window creation
- OpenGL context management
- Event polling

#### Input System (`Core/Input/`)

**Input.hpp/cpp** - Global input polling
- Keyboard state queries (`IsKeyPressed`)
- Mouse button state (`IsMouseButtonPressed`)
- Mouse position tracking (`GetMousePosition`, `GetMouseDelta`)
- Must be initialized with `Input::Init(window)`

**ViewportInput.hpp/cpp** - Viewport-scoped input
- Tracks viewport bounds within ImGui
- Determines if mouse is inside viewport
- Manages camera activation state
- Provides viewport-relative mouse delta
- Handles cursor lock/unlock for camera mode

See [INPUT_SYSTEM.md](INPUT_SYSTEM.md) for detailed documentation.

---

### Rendering Subsystem

**Location:** `Engine/Rendering/`

#### Renderer (`Renderer.hpp/cpp`)
High-level rendering API:
- `Init()` - Initialize OpenGL state (depth testing, etc.)
- `BeginScene(viewProj)` - Start a render pass with camera matrix
- `Submit(mesh, transform, shader)` - Submit a mesh for rendering
- `EndScene()` - Complete the render pass
- `Clear(color)` - Clear framebuffer

#### Mesh System (`Rendering/Mesh/`)

**Mesh.hpp/cpp** - Mesh abstraction
- Factory methods for primitives:
  - `CreateCube()` - Unit cube centered at origin
  - `CreateTriangle3D()` - Triangle in XZ plane
  - `CreateCircle(segments)` - Circle with configurable tessellation
- Encapsulates `VertexArray` and index count
- Manages geometry lifetime with smart pointers

#### Buffers (`Rendering/Buffers/`)

**VertexBuffer** - GPU vertex data storage
**IndexBuffer** - GPU index data storage
**VertexArray** - VAO wrapper managing vertex/index buffer binding

#### Shaders (`Rendering/Shaders/`)

**Shader.hpp/cpp** - Shader program management
- Compilation from source strings
- Uniform setting (`SetMat4`, `SetVec3`, `SetFloat`, etc.)
- Automatic program binding

#### Framebuffer (`Rendering/Framebuffer/`)

**Framebuffer.hpp/cpp** - Offscreen rendering target
- Creates color and depth attachments
- Used to render scene to ImGui viewport texture
- Resizable to match viewport dimensions

#### Camera (`Rendering/Camera/`)

**EditorCamera.hpp/cpp** - Free-flying editor camera
- Fly-through controls (WASD + mouse look)
- Generates view and projection matrices
- Configurable FOV, near/far planes, and movement speed
- `ProcessKeyboard(direction, deltaTime)` - Apply movement
- `ProcessMouseMovement(xOffset, yOffset)` - Apply rotation

See [RENDERING.md](RENDERING.md) for detailed documentation.

---

### Scene Subsystem

**Location:** `Engine/Scene/`

The scene system implements an Entity-Component-System (ECS) architecture using [EnTT](https://github.com/skypjack/entt).

#### Scene (`Scene.hpp/cpp`)
- Manages the EnTT registry
- Entity creation and destruction
- Provides access to registry for component queries
- `CreateEntity(name)` - Create new entity with unique ID

#### Entity (`Entity.hpp`)
- Lightweight wrapper around `entt::entity` handle
- Provides component management interface:
  - `AddComponent<T>(args...)` - Add component to entity
  - `GetComponent<T>()` - Retrieve component reference
  - `HasComponent<T>()` - Check for component existence
  - `RemoveComponent<T>()` - Remove component from entity

#### Components (`Components.hpp`)

**TagComponent** - Human-readable entity name
```cpp
struct TagComponent {
    std::string Tag;
};
```

**TransformComponent** - 3D spatial transformation
```cpp
struct TransformComponent {
    glm::vec3 Position{0.0f};
    glm::vec3 Rotation{0.0f};  // Euler angles (degrees)
    glm::vec3 Scale{1.0f};
    
    glm::mat4 GetMatrix() const;  // Computes model matrix
};
```

**MeshComponent** - Renderable mesh reference
```cpp
struct MeshComponent {
    std::shared_ptr<Mesh> MeshHandle;
};
```

#### SceneAPI (`SceneAPI.hpp`)
Helper functions for common scene operations:
- `CreateCamera(scene, name)` - Create camera entity
- `CreateMeshEntity(scene, name, mesh, position)` - Create renderable entity
- `CreateDefaultScene(scene)` - Populate scene with starting content

See [SCENE_SYSTEM.md](SCENE_SYSTEM.md) for detailed documentation.

---

## Editor Module

**Location:** `Editor/`

The editor is a standalone executable (`UICheckEditor`) that uses the engine library.

### Editor Entry Point (`main.cpp`)

The editor uses the engine's entry point system. `main()` is defined in the engine, which calls the client's `CreateApplication` factory function.

```cpp
#include "Core/EditorApplication.hpp"
#include <Core/EntryPoint.hpp> // Defines main()

Application* CreateApplication(ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
    spec.Name = "Groove Engine Editor";
    // ...
    return new EditorApplication(spec);
}
```

### Editor Application (`Core/EditorApplication.hpp/cpp`)

Inherits from `Application` and manages the editor's lifecycle:

```cpp
class EditorApplication : public Application
{
public:
    virtual void OnInit() override {
        // Initialize Input, Layers
    }
    
    virtual void OnUpdate(float deltaTime) override {
        // Clear screen
        // Update input
        // Render ImGui layers
    }
    
    virtual void OnShutdown() override {
        // Cleanup
    }
};
```

### ImGuiLayer (`Core/ImGuiLayer.hpp/cpp`)

Responsibilities:
- Initialize ImGui with GLFW and OpenGL3 backends
- Configure ImGui for docking (`ImGuiConfigFlags_DockingEnable`)
- Start and end ImGui frames
- Handle multiple viewports
- Apply custom dark theme styling

Key config flags:
- `ImGuiConfigFlags_DockingEnable` - Enable docking
- `ImGuiConfigFlags_ViewportsEnable` - Multi-viewport support
- `ImGuiConfigFlags_NoMouseCursorChange` - Let viewport input manage cursor
- `ImGuiConfigFlags_DpiEnableScaleFonts` - DPI-aware font scaling
- `ImGuiConfigFlags_DpiEnableScaleViewports` - DPI-aware viewport scaling

**Default Engine Style:**
- Custom dark theme defined in `OnAttach()`
- Stored in `ImGuiLayer::DefaultEngineStyle` before applying user themes
- Enables theme restoration when "Use Default Theme" is selected

---

### Theme System

The engine includes a complete runtime theme customization system.

#### ThemeSettings (`Core/ThemeSettings.hpp/cpp`)

Static class managing theme loading and persistence:

```cpp
class ThemeSettings {
public:
    static void Init();              // Load at engine startup
    static void ApplyThemeFromJSON(); // Apply JSON overrides to ImGui style
    static void SaveThemeToJSON();   // Write current style to JSON file
    static bool UseDefaultTheme;     // Flag controlled by UI
};
```

**Initialization Flow:**
1. `ImGuiLayer::OnAttach()` sets up default engine theme
2. Stores `DefaultEngineStyle = ImGui::GetStyle()`
3. Calls `ThemeSettings::Init()` to load JSON file
4. If not using default, calls `ThemeSettings::ApplyThemeFromJSON()`

**JSON File Location:** `settings/theme/params.json`

**JSON Structure:**
```json
{
    "UseDefaultTheme": false,
    "WindowRounding": 8.0,
    "FrameRounding": 8.0,
    "TabRounding": 3.0,
    "WindowPadding": [8, 8],
    "FramePadding": [6, 4],
    "ItemSpacing": [8, 6],
    "Colors": {
        "WindowBg": [0.11, 0.11, 0.12, 1.0],
        "FrameBg": [0.16, 0.16, 0.18, 1.0],
        ...
    }
}
```

**Key Features:**
- Automatic directory creation (`settings/theme/`)
- Safe JSON parsing with error handling
- Support for RGB and RGBA color arrays
- All ImGui style colors supported via `ImGui::GetStyleColorName()`

#### Default Theme Restoration

When "Use Default Theme" is selected in the Theme Panel:
```cpp
ImGui::GetStyle() = ImGuiLayer::DefaultEngineStyle;
```

This restores the exact engine-defined theme, not ImGui's built-in dark theme.

### EditorLayer (`Core/EditorLayer.hpp/cpp`)

Main editor interface layer:

**Setup:**
- Creates active scene with default content
- Initializes framebuffer for viewport rendering
- Loads shaders for mesh rendering
- Initializes editor camera

**Rendering Loop:**
1. Bind framebuffer
2. Clear to background color
3. Render scene:
   - `Renderer::BeginScene(camera.GetViewProjection())`
   - Query all entities with `TransformComponent` + `MeshComponent`
   - Submit each mesh with `Renderer::Submit(mesh, transform, shader)`
   - `Renderer::EndScene()`
4. Unbind framebuffer

**ImGui Interface Panels:**

1. **Viewport Panel** (`DrawViewportPanel()`)
   - Displays framebuffer color attachment texture
   - Handles viewport resizing with framebuffer recreation
   - Updates `ViewportInput` bounds for camera controls
   - Supports ImGui window flags for no-scrolling

2. **Hierarchy Panel** (`DrawHierarchyPanel()`)
   - Lists all entities in active scene using `registry.view<TagComponent>()`
   - Displays entity names from TagComponent
   - Click to select entity (sets `m_SelectedEntity`)
   - Visual highlight for selected entity

3. **Inspector Panel** (`DrawInspectorPanel()`)
   - Shows components of `m_SelectedEntity`
   - Editable fields for:
     - **TagComponent**: Entity name text input
     - **TransformComponent**: Position, Rotation, Scale (DragFloat3)
   - Component-based sections with collapsible headers

4. **Content Browser Panel** (`DrawContentBrowserPanel()`)
   - Placeholder for future asset management
   - Will support file browsing and asset import

5. **Theme Panel** (`DrawThemePanel()`)
   - Toggled via `m_ShowThemePanel` flag
   - Access from Settings menu
   - See Theme System section below

---

## Data Flow: Rendering a Frame

```
1. Main Loop (main.cpp)
   └─► Update Camera Input
       └─► ViewportInput checks mouse in viewport
           └─► Process WASD + mouse delta
               └─► EditorCamera updates view matrix

2. EditorLayer::OnImGuiRender()
   └─► Render to Framebuffer
       ├─► Renderer::BeginScene(viewProj)
       ├─► Query ECS: view<TransformComponent, MeshComponent>()
       ├─► For each entity:
       │   ├─► Get transform matrix
       │   ├─► Get mesh reference
       │   └─► Renderer::Submit(mesh, transform, shader)
       │       ├─► Shader sets uniforms (u_ViewProj, u_Model)
       │       ├─► Mesh VAO binds
       │       └─► glDrawElements()
       └─► Renderer::EndScene()
   
   └─► Render ImGui Viewport
       ├─► Display framebuffer texture
       └─► Update ViewportInput bounds

3. ImGuiLayer::End()
   └─► ImGui::Render()
       └─► Draw UI to screen
```

---

## Build System

The project uses CMake with a hierarchical structure:

**Root CMakeLists.txt:**
- Sets up vendor libraries (GLAD, GLFW, ImGui)
- Includes GLM, EnTT, STB headers
- Adds Engine and Editor subdirectories

**Engine/CMakeLists.txt:**
- Compiles `UICheckEngine` static library
- Links GLAD, GLFW, OpenGL

**Editor/CMakeLists.txt:**
- Compiles `UICheckEditor` executable
- Links `UICheckEngine` and `imgui`

See [BUILD.md](BUILD.md) for build instructions.

---

## Key Design Patterns

### Entity-Component-System (ECS)
- **Entities** are IDs, not classes
- **Components** are plain data structures
- **Systems** are functions that query and process components
- Provides data-oriented design for performance and flexibility

### Resource Ownership
- **Smart pointers** for automatic cleanup (Scene owns entities, MeshComponent owns mesh)
- **Unique ownership** for framebuffers, shaders (EditorLayer)
- **Shared ownership** for meshes (multiple entities can reference same mesh)

### Layer Architecture
- Modular attachment system (ImGuiLayer, EditorLayer)
- Each layer handles specific responsibilities
- Easy to add new layers (e.g., PhysicsLayer, AudioLayer)

---

## Threading Model

Currently **single-threaded**:
- Main thread handles window events, input, rendering, and ImGui
- All OpenGL calls on main thread (required)

Future multi-threading could include:
- Asset loading on background threads
- Physics simulation on separate thread
- Audio processing on dedicated thread

---

## Performance Considerations

### Current Optimizations
- Static vendor libraries (compiled once)
- Indexed rendering (reduces vertex duplication)
- Framebuffer caching (reuses same FBO, only resizes when needed)
- EnTT's sparse set for fast component queries

### Future Optimizations
- Batched rendering (reduce draw calls)
- Frustum culling (skip offscreen entities)
- LOD system (simpler meshes at distance)
- Material instancing (group by shader/texture)

---

## Extension Points

The architecture is designed for extensibility:

**New Components:**
Add to `Components.hpp`, use immediately in systems

**New Render Systems:**
Implement in `Rendering/` and register with `Renderer`

**Custom Editor Panels:**
Add to `EditorLayer::OnImGuiRender()`

**Asset Importers:**
Create in `Engine/Assets/` (future)

---

## Dependencies Summary

| Library | Purpose | Integration |
|---------|---------|-------------|
| **GLFW** | Window + Input | Submodule in vendor/ |
| **GLAD** | OpenGL Loader | Generated, in vendor/ |
| **GLM** | Math (vectors, matrices) | Header-only, vendor/ |
| **ImGui** | Editor UI | Compiled to static lib |
| **EnTT** | ECS Registry | Single header, vendor/ |
| **STB** | Image loading (future) | Header-only, vendor/ |

All dependencies are vendored—no external installations required.
