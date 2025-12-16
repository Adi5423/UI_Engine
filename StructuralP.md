# Engine Code Quality & Architecture Review

## Overview
This document outlines issues, bugs, and areas for improvement found during a comprehensive scan of the codebase. The comparison standard is set against professional game engines like Unreal Engine and Unity, as well as modern C++ game engine architecture best practices (reference: Hazel, Godot).

## 1. Critical Architectural Issues

### 1.1 Rendering Logic Coupled with UI
**File:** `Editor/Core/EditorLayer.cpp` (Lines 498-536) & `DrawViewportPanel`
- **Issue:** The `EditorLayer` directly issues OpenGL commands (`glDrawElements`, `glEnable`, `glPolygonMode`). Use of raw `glad` calls in the Editor layer breaks the "Renderer" abstraction.
- **Why it's bad:** The Editor should not know *how* to render the scene, only *that* it should be rendered. This makes it impossible to implement advanced rendering features (shadows, post-processing, batching) without rewriting the Editor.
- **Professional Approach:** The `EditorLayer` should call `m_ActiveScene->OnRender(camera)` or pass the Render Pass to a `SceneRenderer`.

### 1.2 Input & Raycasting Performance
**File:** `Editor/Core/EditorLayer.cpp` (Lines 25-59, 616-634)
- **Issue:** Raycasting (mouse picking) is implemented via a brute-force O(N) linear search over all entities in the UI loop (`OnImGuiRender`).
- **Why it's bad:** This will cause massive lag as the scene grows. Calculation is done on the CPU using a custom `RayIntersectsAABB` function.
- **Professional Approach:**
    1.  Move Physics/Collision logic to a `PhysicsSystem`.
    2.  Use a spatial partition (BVH, Octree) or a Physics Engine (PhysX, Jolt, Bullet) for queries.
    3.  Alternatively, use "ID Buffers" (render entity IDs to a texture) for pixel-perfect selection with O(1) complexity.

### 1.3 Lack of Serialization
**File:** `Engine/Scene/*`
- **Issue:** There is no mechanism to Save or Load scenes.
- **Why it's bad:** A game engine without persistence is unusable for production.
- **Professional Approach:** Implement a serializer (YAML or JSON) to save the Entity Component System state to disk.

## 2. Code Quality & Implementation Details

### 2.1 Hardcoded Shaders
**File:** `Editor/Core/EditorLayer.cpp` (Lines 72-90)
- **Issue:** GLSL Shader source code is hardcoded as C++ strings in `OnAttach`.
- **Why it's bad:** Prevents hot-reloading, lacks syntax highlighting, and bloats the C++ file.
- **Professional Approach:** Load shaders from `.glsl` files via an Asset Manager.

### 2.2 Hardcoded Framebuffer Size
**File:** `Editor/Core/EditorLayer.cpp` (Line 71)
- **Issue:** `m_Framebuffer = std::make_unique<Framebuffer>(1280, 720);`
- **Why it's bad:** Assumes specific start resolution. If the window opens smaller/larger, it might cause artifacts until the first resize event.
- **Professional Approach:** Defer creation until the first update or pass the initial viewport specs properly.

### 2.3 Asset Management
**File:** `Editor/Core/EditorLayer.cpp` (Lines 443-445)
- **Issue:** "Content Browser" is just a list of dummy `Selectable` text items.
- **Why it's bad:** No way to drag-and-drop assets or browse the file system.
- **Professional Approach:** Implement an Asset Manager that scans the `assets` directory and assigns GUIDs to files.

### 2.4 Entity Encapsulation
**File:** `Editor/Core/EditorBridge.cpp`
- **Issue:** `EditorBridge` relies on a "hack" or assumption that `Entity` exposes its internal Scene pointer, or requires modifying `Entity` to expose it (Breaking Encapsulation).
- **Professional Approach:** `Entity` should be a lightweight handle. The `Scene` should be the source of truth.

## 3. Bugs & Potential Crashes

### 3.1 Unsafe Enum Casting
**File:** `Editor/Core/EditorLayer.cpp` (Line 556)
- **Issue:** `(ImGuizmo::OPERATION)m_GizmoType`
- **Risk:** If `m_GizmoType` is -1 (which it is set to when Q is pressed), casting it to an Enum might invoke Undefined Behavior inside `ImGuizmo::Manipulate` or just do nothing (lucky case).
- **Fix:** Check `if (m_GizmoType != -1)` before calling Manipulate (which is done, but the logic flow is fragile).

### 3.2 Viewport Resize Conflict
**File:** `Engine/Core/Application.cpp` vs `EditorLayer.cpp`
- **Issue:** `Application::OnWindowResize` calls `glViewport`.
- **Conflict:** The Editor uses a Framebuffer. If `Application` changes global `glViewport`, it might override the Framebuffer's viewport settings depending on call order.
- **Fix:** The `Renderer` should manage viewport state, not the Window event callback directly.

## 4. Optimization Opportunities

*   **Batch Rendering:** Currently uses one draw call per entity (`glDrawElements` inside a loop). This is extremely slow. Should use Batch Rendering (merging geometry) or Instancing.
*   **Memory Allocations:** `ImGui` strings and `std::string` usage in `OnUpdate` (e.g., `DrawInspectorPanel` text buffers) can be optimized.

## 5. Missing Professional Features
*   **Physics Integration:** No RigidBody, BoxCollider, etc.
*   **Scripting:** No C#/Lua scripting layer.
*   **Material System:** Everything renders with the same hardcoded shader (Lines 72-90 in EditorLayer).
