# Input System

The UI Engine features a dual-architecture input system designed to provide both global input polling and viewport-scoped camera controls for the editor.

---

## Overview

The input system consists of two complementary classes:

1. **`Input`** - Global input polling for keyboard and mouse state
2. **`ViewportInput`** - Viewport-aware input management for editor camera control

This separation ensures that camera controls only activate when interacting with the 3D viewport, not when clicking on UI panels.

---

## Input Class

**Location:** `Engine/Core/Input/Input.hpp/cpp`

### Purpose

Provides simple, stateless polling of keyboard and mouse input anywhere in the application.

### Initialization

Must be initialized before use:

```cpp
GLFWwindow* window = /* ... */;
Input::Init(window);
```

### API

#### Keyboard Input

```cpp
bool Input::IsKeyPressed(int key);
```

**Example:**
```cpp
if (Input::IsKeyPressed(GLFW_KEY_W)) {
    // Move forward
}

if (Input::IsKeyPressed(GLFW_KEY_ESCAPE)) {
    // Exit application
}
```

#### Mouse Button Input

```cpp
bool Input::IsMouseButtonPressed(int button);
```

**Example:**
```cpp
if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
    // Fire weapon / Select entity
}

if (Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
    // Context menu / Camera look
}
```

#### Mouse Position

```cpp
void Input::GetMousePosition(double& x, double& y);
```

Returns absolute mouse position in window coordinates (origin at top-left).

**Example:**
```cpp
double mouseX, mouseY;
Input::GetMousePosition(mouseX, mouseY);
std::cout << "Mouse at: " << mouseX << ", " << mouseY << std::endl;
```

#### Mouse Delta

```cpp
void Input::GetMouseDelta(double& dx, double& dy);
```

Returns mouse movement since last call. Y is inverted (up = positive).

**Example:**
```cpp
double dx, dy;
Input::GetMouseDelta(dx, dy);
camera.Rotate(dx * sensitivity, dy * sensitivity);
```

**First-Frame Handling:**
On the very first call after `Init()`, delta is set to `(0, 0)` to avoid a large jump.

### Implementation Details

**Static State:**
```cpp
static GLFWwindow* s_Window;
static double s_LastX, s_LastY;
static bool s_First;
```

**Delta Calculation:**
```cpp
void Input::GetMouseDelta(double& dx, double& dy) {
    double x, y;
    glfwGetCursorPos(s_Window, &x, &y);
    
    if (s_First) {
        s_LastX = x;
        s_LastY = y;
        s_First = false;
    }
    
    dx = x - s_LastX;
    dy = s_LastY - y;  // Invert Y-axis
    s_LastX = x;
    s_LastY = y;
}
```

---

## ViewportInput Class

**Location:** `Engine/Core/Input/ViewportInput.hpp/cpp`

### Purpose

Manages viewport-scoped camera activation and provides mouse delta tracking that:
- Only activates when mouse is **inside the viewport bounds**
- Manages cursor locking/unlocking
- Prevents camera movement when interacting with ImGui UI panels

### Initialization

Must be initialized after `Input`:

```cpp
GLFWwindow* window = /* ... */;
ViewportInput::Init(window);
```

### Setting Viewport Bounds

The viewport bounds must be updated whenever the ImGui viewport panel changes position or size:

```cpp
void ViewportInput::SetViewportBounds(float x, float y, float width, float height);
```

**Example (in EditorLayer):**
```cpp
ImVec2 windowPos = ImGui::GetWindowPos();
ImVec2 cursorPos = ImGui::GetCursorPos();
ImVec2 globalImage = { windowPos.x + cursorPos.x, windowPos.y + cursorPos.y };

ViewportInput::SetViewportBounds(
    globalImage.x,
    globalImage.y,
    viewportWidth,
    viewportHeight
);
```

> **Important:** Use the **image position** (where the framebuffer texture starts), not just the window position. Account for ImGui padding and title bars.

### Camera State Management

#### Updating Camera State

Called every frame to determine if camera should be active:

```cpp
void ViewportInput::UpdateCameraState(bool rightMousePressed);
```

**Logic:**
1. **Enter Camera Mode:** If RMB pressed AND mouse inside viewport → activate camera, lock cursor
2. **Exit Camera Mode:** If RMB released → deactivate camera, unlock cursor
3. **Keep Active:** If already active and RMB still held → remain active (no cursor mode changes)

**Example (in main.cpp):**
```cpp
ViewportInput::UpdateCameraState(
    Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)
);
```

#### Checking Camera State

```cpp
bool ViewportInput::IsCameraActive();
```

Returns `true` if camera controls should be processed.

**Example:**
```cpp
if (ViewportInput::IsCameraActive()) {
    // Process WASD movement
    // Process mouse look
}
```

### Mouse Delta (Viewport-Scoped)

```cpp
void ViewportInput::GetMouseDelta(double& dx, double& dy);
```

Returns mouse movement delta, **only valid when camera is active**.

**Differences from `Input::GetMouseDelta()`:**
- Resets on camera activation to prevent initial jump
- Manages its own `s_First` flag
- Should be used for viewport camera movement

**Example:**
```cpp
if (ViewportInput::IsCameraActive()) {
    double dx, dy;
    ViewportInput::GetMouseDelta(dx, dy);
    
    editorCamera.ProcessMouseMovement(dx, dy);
}
```

### Checking Mouse in Viewport

```cpp
bool ViewportInput::IsMouseInsideViewport();
```

Returns `true` if current mouse position is within viewport bounds.

**Useful for:**
- Highlighting viewport when hovering
- Detecting viewport clicks for entity selection
- Debug visualization

---

## Usage Pattern: Editor Camera Controls

**Location:** `Editor/main.cpp`

### Setup

```cpp
// Initialize both input systems
GLFWwindow* window = app.GetWindow()->GetNativeWindow();
Input::Init(window);
ViewportInput::Init(window);
```

### Main Loop

```cpp
while (running) {
    // 1. Update camera activation state
    ViewportInput::UpdateCameraState(
        Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)
    );
    
    // 2. If camera active, process input
    if (ViewportInput::IsCameraActive()) {
        // Delta time for smooth movement
        static float lastTime = (float)glfwGetTime();
        float time = (float)glfwGetTime();
        float dt = time - lastTime;
        lastTime = time;
        
        // WASD movement direction
        glm::vec3 dir{0.0f};
        if (Input::IsKeyPressed(GLFW_KEY_W)) dir.z += 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_S)) dir.z -= 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_A)) dir.x -= 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_D)) dir.x += 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_E)) dir.y += 1.0f;
        if (Input::IsKeyPressed(GLFW_KEY_Q)) dir.y -= 1.0f;
        
        // Mouse look
        double dx, dy;
        ViewportInput::GetMouseDelta(dx, dy);
        
        // Apply to camera
        editor.GetCamera().ProcessKeyboard(dir, dt);
        editor.GetCamera().ProcessMouseMovement((float)dx, (float)dy);
    }
    
    // 3. Render and update UI
    // ...
}
```

### ImGui Integration

In `ImGuiLayer::OnAttach()`, disable ImGui's cursor management:

```cpp
ImGuiIO& io = ImGui::GetIO();
io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
```

This allows `ViewportInput` to control cursor visibility with `glfwSetInputMode()`.

---

## State Machine: Camera Activation

```
┌──────────────┐
│   INACTIVE   │ (Cursor: Normal)
│              │
│ Camera: OFF  │
└──────┬───────┘
       │
       │ RMB pressed + Mouse in viewport
       ▼
┌──────────────┐
│   ACTIVE     │ (Cursor: Disabled/Locked)
│              │
│ Camera: ON   │
└──────┬───────┘
       │
       │ RMB released
       ▼
┌──────────────┐
│   INACTIVE   │ (Cursor: Normal)
└──────────────┘
```

**Transitions:**
- **INACTIVE → ACTIVE:** RMB pressed AND `IsMouseInsideViewport()` returns true
- **ACTIVE → INACTIVE:** RMB released (regardless of mouse position)

**Cursor States:**
- **INACTIVE:** `GLFW_CURSOR_NORMAL` (visible, free movement)
- **ACTIVE:** `GLFW_CURSOR_DISABLED` (hidden, infinite movement for mouselook)

---

## Implementation Details

### ViewportInput Static State

```cpp
static GLFWwindow* s_Window;
static glm::vec4 s_ViewportBounds{0, 0, 1280, 720};  // x, y, width, height
static bool s_CameraActive = false;
static double s_LastX = 0.0, s_LastY = 0.0;
static bool s_First = true;
```

### Camera Activation Logic

```cpp
void ViewportInput::UpdateCameraState(bool rightMousePressed) {
    bool inside = IsMouseInsideViewport();
    
    // --- ENTER CAMERA MODE (one time) ---
    if (!s_CameraActive && rightMousePressed && inside) {
        s_CameraActive = true;
        s_First = true;   // Reset delta
        glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        return;
    }
    
    // --- EXIT CAMERA MODE ---
    if (s_CameraActive && !rightMousePressed) {
        s_CameraActive = false;
        glfwSetInputMode(s_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        return;
    }
    
    // --- KEEP CAMERA MODE ACTIVE ---
    // Do NOT switch cursor modes here (prevents flicker)
}
```

**Key Design Decisions:**
1. **Early returns** prevent multiple cursor mode changes in one frame
2. **Reset `s_First`** when entering camera mode prevents initial jump
3. **No cursor changes** when keeping camera active (avoids flicker)

---

## Edge Cases Handled

### Cursor Leaves Viewport While Active

**Behavior:** Camera remains active. User can still move camera even if mouse leaves viewport bounds.

**Rationale:** Prevents frustration when making large mouse movements. User must release RMB to deactivate.

### Window Loses Focus

**GLFW Behavior:** Input states reset, RMB registers as released.

**Result:** `UpdateCameraState()` detects release and deactivates camera.

### Viewport Resized Mid-Camera

**Handling:** Bounds updated via `SetViewportBounds()`, but camera remains active if RMB still held.

### Multiple Viewports (Future)

**Current:** Single global viewport bounds.

**Future:** Per-viewport tracking with viewport IDs.

---

## Debugging Tips

### Camera Not Activating

**Check:**
1. Is `ViewportInput::Init()` called?
2. Are viewport bounds set correctly? (Add debug print)
3. Is mouse actually inside bounds? (Check `IsMouseInsideViewport()`)

**Debug Code:**
```cpp
ImGui::Text("VP Bounds: X=%.0f Y=%.0f W=%.0f H=%.0f",
    s_ViewportBounds.x, s_ViewportBounds.y,
    s_ViewportBounds.z, s_ViewportBounds.w);

double mx, my;
Input::GetMousePosition(mx, my);
ImGui::Text("Mouse: %.0f, %.0f | Inside: %s",
    mx, my, ViewportInput::IsMouseInsideViewport() ? "YES" : "NO");
```

### Cursor Flickers

**Cause:** Multiple `glfwSetInputMode()` calls per frame.

**Fix:** Ensure `UpdateCameraState()` logic uses early returns.

### Large Initial Mouse Jump

**Cause:** `s_First` not reset on camera activation.

**Fix:** Set `s_First = true;` when entering camera mode.

---

## Future Enhancements

### Multi-Viewport Support

Track active viewport ID:
```cpp
static int s_ActiveViewportID = -1;

void ViewportInput::UpdateCameraState(int viewportID, bool rightMousePressed);
```

### Configurable Activation

Allow different activation keys:
```cpp
static int s_CameraActivationButton = GLFW_MOUSE_BUTTON_RIGHT;

void ViewportInput::SetActivationButton(int button);
```

### Mouse Sensitivity

Per-viewport sensitivity:
```cpp
static float s_MouseSensitivity = 1.0f;

void ViewportInput::SetMouseSensitivity(float sensitivity);
void ViewportInput::GetMouseDelta(double& dx, double& dy) {
    // Apply sensitivity
    dx *= s_MouseSensitivity;
    dy *= s_MouseSensitivity;
}
```

---

## Related Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md) - System overview
- [Camera System](RENDERING.md#editor-camera) - EditorCamera implementation
- [ImGui Integration](ARCHITECTURE.md#editor-module) - Editor layer details
