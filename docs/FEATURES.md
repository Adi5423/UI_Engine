# Features List

The UI Engine provides a robust set of features designed for professional game development and tool creation.

---

## Core Systems

### 1. Professional Entry Point
- **Architecture**: Inversion of Control pattern where the engine owns `main()`.
- **Application Class**: Base class for client applications defining lifecycle (`OnInit`, `OnUpdate`, `OnShutdown`).
- **Separation of Concerns**: Clean boundary between Engine core and Client (Editor/Game) logic.

### 2. Event System
- **Event Bus**: Robust messaging system for decoupling interactions.
- **Event Types**: 
    - **Application**: Window Resize, Window Close
    - **Keyboard**: Key Pressed, Key Released
    - **Mouse**: Moved, Scrolled, Button Pressed/Released
- **Dispatching**: `EventDispatcher` mechanics to route events to appropriate handlers.

### 3. Logging System
- **Console Logging**: Color-coded output for different log levels (Trace, Info, Warn, Error, Fatal).
- **File Logging**: Automatic persistent logging to `logs/engine.log`.
- **Dual Channels**: Separate `CORE` (Engine) and `CLIENT` (App) loggers.

### 4. Resource Management (Backbone)
- **ResourceManager**: Unified efficient loader and cache for assets.
- **Backbone Implementation**: Structured to handling Shaders and Textures.
- **Caching**: Prevents duplicate loading of resources.

### 5. Input System (Hybrid)
- **Global Polling**: `Input::IsKeyPressed` for immediate state checks.
- **Event-Based**: React to state changes via the Event System.
- **Viewport-Scoped**: Special handling for Editor Viewport input (preventing clicks when interacting with UI).

---

## Rendering & Graphics

### 6. Modern Renderer
- **API**: OpenGL 4.6 Core Profile.
- **Abstraction**: `Renderer` class abstracting raw OpenGL calls.
- **Buffers**: VertexBuffers, IndexBuffers, VertexArrays managed automatically.
- **Shaders**: Shader program compilation and uniform management.

### 7. Editor Camera
- **Fly-Camera**: WASD + Mouse Look controls.
- **Viewport Integration**: Only active when focused on the viewport.

### 8. Framebuffer Support
- **Off-screen Rendering**: Scene renders to a framebuffer.
- **Texture Output**: Framebuffer color attachment used as a texture in ImGui.

---

## Scene & ECS

### 9. Entity Component System
- **Framework**: Powered by **EnTT** (fastest C++ ECS).
- **Entities**: Lightweight IDs.
- **Components**: Plain data structs (Transform, Mesh, Tag, etc.).
- **Scene**: Registry manager for all entities.

---

## Editor & UI

### 10. ImGui Editor Interface
- **Docking**: Fully dockable window layout.
- **Panels**:
    - **Viewport**: 3D Scene view.
    - **Hierarchy**: Scene tree view.
    - **Inspector**: Component editing.
    - **Content Browser**: Asset exploration (UI stub).
    - **Themes**: Live theme editor.

### 11. Theme System
- **Runtime Customization**: Edit colors, rounding, and spacing live.
- **Serialization**: Save/Load themes to/from JSON.
- **Persistence**: Remembers last active theme on startup.

---

## Build System

### 12. CMake
- **Cross-Platform**: Supports Windows, Linux, macOS.
- **Dependency Management**: Vendored libraries (GLFW, GLAD, ImGui, GLM, EnTT, etc.).
- **Targets**: Modular build (`UICheckEngine` lib, `UICheckEditor` exe).
