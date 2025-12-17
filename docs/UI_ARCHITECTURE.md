# UI Architecture - Professional Game Engine Design

## Overview

The UI Engine follows professional game engine design patterns inspired by industry-standard engines like Unity, Unreal Engine, and Godot. This document outlines the UI architecture, panel design, and best practices implemented.

## Core Principles

### 1. **Separation of Concerns**
- **EditorLayer**: Manages the overall editor state and panel orchestration
- **Panel System**: Each panel (Inspector, Hierarchy, Content Browser, Viewport) is self-contained
- **Bridge Pattern**: `EditorBridge` decouples UI actions from scene operations

### 2. **Immediate Mode GUI (ImGui)**
- Utilizes Dear ImGui for rapid UI development
- Custom styling and theming for professional appearance
- Persistent state management through `imgui.ini`

### 3. **Command Pattern for Undo/Redo**
- All editor actions go through `CommandHistory`
- Commands are reversible and replayable
- Maintains editor state consistency

## Panel Architecture

### Inspector Panel

The Inspector displays properties of the selected entity with a Unity-like interface:

#### Features:
- **Component-based Display**: Uses `ImGui::CollapsingHeader` for organized component sections
- **Property Controls**: Custom `DrawVec3Control` function for Position/Rotation/Scale
  - Color-coded axis buttons (Red X, Green Y, Blue Z)
  - Reset to default functionality
  - Drag controls with precision input
- **Add Component**: Dropdown for adding new components
- **Undo/Redo Integration**: All property changes are tracked

#### Design Pattern:
```cpp
DrawVec3Control(label, values, resetValue, columnWidth, onStartEdit, onEndEdit)
```
- Lambda callbacks for edit state tracking
- Two-column layout (Label | Controls)
- Multi-item width distribution

### Hierarchy Panel (Scene Graph)

Displays the scene entity tree structure similar to Unity's Hierarchy:

#### Features:
- **Tree View**: Uses `ImGuiTreeNodeFlags` for proper tree rendering
- **Selection Highlighting**: Full-width selection with `SpanAvailWidth` flag
- **Context Menus**: Right-click for entity operations (Delete, Create)
- **Drag & Drop**: Foundation for entity reparenting (future)

#### Best Practices:
- Leaf nodes for entities without children
- Visual feedback on selection
- Keyboard navigation support

### Content Browser Panel

Asset management panel inspired by Unreal Engine's Content Browser:

#### Features:
- **Grid Layout**: Dynamic column count based on panel width
- **Thumbnail Display**: `ImGui::ImageButton` for asset icons
- **Drag & Drop**: Assets can be dragged to viewport/hierarchy
- **Scalable Thumbnails**: User-adjustable thumbnail size
- **Asset Types**: Visual distinction between different file types

#### Implementation Details:
```cpp
columnCount = panelWidth / (thumbnailSize + padding)
ImGui::Columns(columnCount, 0, false)
```

### Viewport Panel

The main 3D rendering viewport with professional editing tools:

#### Features:
- **Framebuffer Integration**: Off-screen rendering for clean compositing
- **Gizmo System**: ImGuizmo for translate/rotate/scale operations
- **Ray Picking**: Click-to-select entities in 3D space
- **Camera Controls**: Viewport-scoped camera navigation
- **Delete Confirmation**: Context-aware deletion popup

## Professional Code Structure

### File Organization

```
UI_Engine/
├── Editor/
│   └── Core/
│       ├── EditorLayer.cpp/hpp     # Main editor orchestration
│       ├── EditorBridge.cpp/hpp    # Command pattern bridge
│       ├── ImGuiLayer.cpp/hpp      # ImGui initialization
│       └── ThemeSettings.cpp/hpp   # Theme management
├── Engine/
│   ├── Rendering/
│   │   ├── SceneRenderer.cpp/hpp   # Rendering abstraction
│   │   └── ...
│   ├── Scene/
│   │   ├── Scene.cpp/hpp           # ECS scene management
│   │   ├── Entity.cpp/hpp          # Entity wrapper
│   │   └── Components.hpp          # Component definitions
│   └── Core/
│       ├── Commands/               # Command pattern implementation
│       └── Input/                  # Input system
└── vendor/                         # Third-party dependencies
```

### Architectural Decisions

1. **ECS (Entity Component System)**
   - Uses EnTT library for high-performance entity management
   - Components are POD (Plain Old Data) structures
   - Systems are decoupled from entity storage

2. **Rendering Pipeline**
   - `SceneRenderer` handles all rendering logic
   - Framebuffer-based viewport for flexibility
   - Separate wireframe pass for selection highlighting

3. **State Management**
   - Editor state is transient (in-memory)
   - Scene state is serializable (future)
   - Theme state persists to JSON

## Performance Considerations

### Optimization Strategies

1. **Deferred Operations**
   - Entity deletion happens after iteration
   - Command execution is batched
   - Rendering is framebuffer-based

2. **Memory Management**
   - Smart pointers for automatic resource cleanup
   - Shared ownership for renderer and scene
   - Stack allocation for temporary UI state

3. **Input Handling**
   - Viewport-scoped input to avoid conflicts
   - Keyboard shortcuts only when appropriate
   - Mouse picking with early-out optimizations

### Profiling Points

- Frame time monitoring (future)
- Draw call batching (future)
- Memory allocation tracking (future)

## UI Standards & Conventions

### Naming Conventions

- **Panels**: `Draw[Name]Panel()` (e.g., `DrawInspectorPanel()`)
- **Controls**: `Draw[Type]Control()` (e.g., `DrawVec3Control()`)
- **State Variables**: `m_[Name]` (e.g., `m_SelectedEntity`)

### ImGui Patterns

1. **Begin/End Pairs**: Always balanced
2. **ID Stack**: Use `PushID/PopID` for uniqueness
3. **Style Stack**: Clean up with `PopStyleColor/PopStyleVar`
4. **Columns**: Always reset to 1 before leaving

### Error Handling

- Null checks for entity validity
- Framebuffer size validation
- Input bounds clamping

## Future Enhancements

### Planned Features

1. **Docking System**: Multi-window layout with save/load
2. **Asset Pipeline**: Import/export for models, textures, scenes
3. **Scripting Integration**: Lua/C# script editing in UI
4. **Profiler Panel**: Real-time performance metrics
5. **Console Panel**: Log messages and command execution
6. **Animation Editor**: Timeline-based animation system

### Scalability

- **Plugin System**: For custom editor tools
- **Theme Engine**: Complete customization of look and feel
- **Localization**: Multi-language support
- **Accessibility**: Keyboard-only navigation, screen reader support

## Testing & Validation

### Build Verification

```bash
cmake --build build --config Debug -j 4
```

### Manual Testing Checklist

- [ ] Entity selection in hierarchy
- [ ] Property editing in inspector with undo/redo
- [ ] Gizmo manipulation in viewport
- [ ] Ray picking accuracy
- [ ] Context menu operations
- [ ] Asset browser navigation
- [ ] Theme settings persistence

## References

- **Unity Editor Architecture**: Component-based inspector, hierarchy tree
- **Unreal Engine UMG**: Asset browser grid layout, viewport controls
- **Godot Editor**: Docking system, theme management
- **Dear ImGui**: Immediate mode GUI best practices

## Conclusion

The UI Engine implements a professional, scalable architecture suitable for game development. The design emphasizes maintainability, performance, and user experience, following industry best practices from leading game engines.
