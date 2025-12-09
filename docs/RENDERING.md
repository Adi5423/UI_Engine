# Rendering System

Documentation for the UI Engine's rendering pipeline, including the mesh system, shaders, buffers, framebuffers, camera, and renderer API.

---

## System Overview

The rendering system is designed around a simple, extensible pipeline:

```
Scene (ECS)
    │
    ▼
┌─────────────────────────────────┐
│   Renderer API                  │
│  - BeginScene(viewProj)         │
│  - Submit(mesh, transform,      │
│           shader)                │
│  - EndScene()                   │
└─────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────┐
│   Mesh System                   │
│  - Vertex Array (VAO)           │
│  - Vertex/Index Buffers         │
└─────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────┐
│   Shader System                 │
│  - Compile programs             │
│  - Set uniforms                 │
└─────────────────────────────────┘
    │
    ▼
┌─────────────────────────────────┐
│   OpenGL                        │
│  - glDrawElements()             │
└─────────────────────────────────┘
```

---

## Renderer API

**Location:** `Engine/Rendering/Renderer.hpp/cpp`

### Purpose

High-level rendering API that abstracts OpenGL calls and manages the render state.

### Initialization

```cpp
Renderer::Init();
```

Called once at startup. Sets up OpenGL state:
- Enables depth testing
- Sets default blend mode (if needed)
- Initializes any global GPU resources

### Render Loop

#### 1. Begin Scene

```cpp
void Renderer::BeginScene(const glm::mat4& viewProj);
```

**Purpose:** Start a render pass with the camera's view-projection matrix.

**Parameters:**
- `viewProj` - Combined view-projection matrix from camera

**Example:**
```cpp
EditorCamera camera;
Renderer::BeginScene(camera.GetViewProjection());
```

**What It Does:**
- Stores `viewProj` in static variable for subsequent `Submit()` calls
- Prepares renderer for accepting draw calls

#### 2. Submit Meshes

```cpp
void Renderer::Submit(const std::shared_ptr<Mesh>& mesh,
                      const glm::mat4& transform,
                      Shader& shader);
```

**Purpose:** Submit a mesh for rendering with a transform and shader.

**Parameters:**
- `mesh` - Mesh containing geometry data (VAO, index count)
- `transform` - Model matrix (object → world space transform)
- `shader` - Shader program to use for rendering

**Example:**
```cpp
auto mesh = Mesh::CreateCube();
glm::mat4 transform = glm::translate(glm::mat4(1.0f), glm::vec3(0, 1, 0));
Renderer::Submit(mesh, transform, shader);
```

**What It Does:**
1. Validates mesh has valid VAO
2. Binds shader program
3. Sets uniforms:
   - `u_ViewProj` ← stored view-projection matrix
   - `u_Model` ← transform parameter
4. Binds mesh VAO
5. Issues draw call: `glDrawElements(GL_TRIANGLES, indexCount, ...)`

#### 3. End Scene

```cpp
void Renderer::EndScene();
```

**Purpose:** Complete the render pass.

**What It Does:**
- Currently a no-op
- Future: Flush batched draw calls, reset state, etc.

### Clear Framebuffer

```cpp
void Renderer::Clear(const glm::vec4& color);
```

**Purpose:** Clear color and depth buffers.

**Parameters:**
- `color` - Clear color (RGBA, 0.0 - 1.0 range)

**Example:**
```cpp
Renderer::Clear({0.12f, 0.12f, 0.14f, 1.0f});  // Dark gray background
```

---

## Mesh System

**Location:** `Engine/Rendering/Mesh/Mesh.hpp/cpp`

### Mesh Class

Abstraction over OpenGL geometry data.

**Members:**
```cpp
class Mesh {
private:
    std::unique_ptr<VertexArray> m_VertexArray;
    uint32_t m_IndexCount;
    PrimitiveType m_Type;
};
```

**Primitive Types:**
```cpp
enum class PrimitiveType {
    None = 0,
    Cube,
    Triangle3D,
    Circle
};
```

### Vertex Structure

All mesh geometry uses the `Vertex` structure:

```cpp
struct Vertex {
    glm::vec3 Position;  // 3D position (x, y, z)
    glm::vec3 Normal;    // Surface normal for lighting
};
```

**Vertex Attributes:**
- **Location 0:** Position (3 floats)
- **Location 1:** Normal (3 floats)

Each vertex is `sizeof(Vertex) = 24 bytes` (6 floats × 4 bytes).

### Internal Mesh Constructor

The private constructor handles all OpenGL setup:

```cpp
Mesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
```

**Parameters:**
- `vertices` - Array of `Vertex` structs containing position and normal data
- `indices` - Array of triangle indices (3 indices per triangle)

**What It Does:**
1. **Creates Vertex Array Object (VAO)**
   ```cpp
   m_VertexArray = std::make_unique<VertexArray>();
   ```

2. **Uploads Vertex Data to GPU**
   ```cpp
   VertexBuffer* vb = new VertexBuffer(vertices.data(), vertices.size() * sizeof(Vertex));
   ```

3. **Uploads Index Data to GPU**
   ```cpp
   IndexBuffer* ib = new IndexBuffer(indices.data(), (uint32_t)indices.size());
   m_IndexCount = (uint32_t)indices.size();
   ```

4. **Configures Vertex Attributes**
   ```cpp
   // Position attribute (location = 0)
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                        (void*)offsetof(Vertex, Position));
   
   // Normal attribute (location = 1)
   glEnableVertexAttribArray(1);
   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                        (void*)offsetof(Vertex, Normal));
   ```

**Vertex Layout:**
```
Offset 0:  Position.x (4 bytes)
Offset 4:  Position.y (4 bytes)
Offset 8:  Position.z (4 bytes)
Offset 12: Normal.x   (4 bytes)
Offset 16: Normal.y   (4 bytes)
Offset 20: Normal.z   (4 bytes)
Total: 24 bytes per vertex
```

---

### Factory Methods

These static methods create predefined mesh primitives.

#### Create Cube

```cpp
static std::shared_ptr<Mesh> Mesh::CreateCube();
```

**Purpose:** Creates a unit cube mesh centered at the origin.

**Parameters:** None

**Returns:** `std::shared_ptr<Mesh>` - Shared pointer to the created cube mesh

**Geometry Specification:**
- **Dimensions:** 1×1×1 unit cube
- **Center:** Origin (0, 0, 0)
- **Bounds:** From (-0.5, -0.5, -0.5) to (0.5, 0.5, 0.5)
- **Vertices:** 24 (4 per face × 6 faces)
- **Triangles:** 12 (2 per face × 6 faces)
- **Indices:** 36 (3 per triangle × 12 triangles)

**Face Layout:**
- Front face (normal = {0, 0, 1})
- Back face (normal = {0, 0, -1})
- Left face (normal = {-1, 0, 0})
- Right face (normal = {1, 0, 0})
- Top face (normal = {0, 1, 0})
- Bottom face (normal = {0, -1, 0})

**Why 24 vertices instead of 8?**  
Each face needs unique normals, so we cannot share vertices between faces. This allows proper lighting calculations.

**Example:**
```cpp
auto cubeMesh = Mesh::CreateCube();

// Create entity with cube
Entity cube = scene->CreateEntity("Cube");
cube.AddComponent<TransformComponent>(glm::vec3(0, 1, 0));
cube.AddComponent<MeshComponent>(cubeMesh);
```

**Common Use Cases:**
- Building blocks for level geometry
- Placeholder objects
- Physics collision boxes
- UI elements (buttons, panels)

---

#### Create Triangle3D (Pyramid)

```cpp
static std::shared_ptr<Mesh> Mesh::CreateTriangle3D();
```

**Purpose:** Creates a 3D pyramid with a square base.

**Parameters:** None

**Returns:** `std::shared_ptr<Mesh>` - Shared pointer to the created pyramid mesh

**Geometry Specification:**
- **Base:** Square on XZ plane (y = 0)
- **Base Size:** 1×1 (from -0.5 to 0.5 on X and Z)
- **Apex Height:** 1.0 unit above base (y = 1.0)
- **Apex Position:** Center of base (x = 0, z = 0)
- **Vertices:** 5 (4 base corners + 1 apex)
- **Triangles:** 6 (2 for base + 4 for sides)
- **Indices:** 18 (3 per triangle × 6 triangles)

**Vertex Positions:**
```cpp
Base corners:
  (-0.5, 0.0, -0.5)  // Back-left
  ( 0.5, 0.0, -0.5)  // Back-right
  ( 0.5, 0.0,  0.5)  // Front-right
  (-0.5, 0.0,  0.5)  // Front-left

Apex:
  (0.0, 1.0, 0.0)    // Top center
```

**Example:**
```cpp
auto pyramidMesh = Mesh::CreateTriangle3D();

// Create a pyramid at position
Entity pyramid = SceneAPI::CreateMeshEntity(*scene, "Pyramid", 
                                           pyramidMesh, 
                                           glm::vec3(5, 0, 0));
```

**Common Use Cases:**
- Directional indicators (arrows, pointers)
- Landmarks in 3D space
- Simple roof structures
- Testing 3D rendering

---

#### Create Circle (Flat Disc)

```cpp
static std::shared_ptr<Mesh> Mesh::CreateCircle(uint32_t segments = 32);
```

**Purpose:** Creates a flat circular disc using a triangle fan topology.

**Parameters:**
- `segments` (optional) - Number of edge segments
  - **Type:** `uint32_t`
  - **Default:** 32
  - **Minimum:** 3 (creates a triangle)
  - **Range:** Typically 3-128
  - **Effect:** Higher values = smoother circle, more triangles

**Returns:** `std::shared_ptr<Mesh>` - Shared pointer to the created circle mesh

**Geometry Specification:**
- **Plane:** XZ plane (y = 0)
- **Radius:** 0.5 units
- **Center:** Origin (0, 0, 0)
- **Normal:** Upward (+Y direction: {0, 1, 0})
- **Topology:** Triangle fan (center + perimeter vertices)

**Vertex Count:** `segments + 2`
- 1 center vertex
- `segments + 1` perimeter vertices (first and last overlap to close the circle)

**Triangle Count:** `segments`

**Index Count:** `segments × 3`

**Algorithm:**
```cpp
// Center vertex at origin
vertices[0] = { {0, 0, 0}, {0, 1, 0} };

// Perimeter vertices
float angleStep = 2π / segments;
for (int i = 0; i <= segments; i++) {
    float angle = angleStep * i;
    float x = cos(angle) * 0.5;
    float z = sin(angle) * 0.5;
    vertices[i+1] = { {x, 0, z}, {0, 1, 0} };
}

// Triangle indices (fan from center)
for (int i = 0; i < segments; i++) {
    indices = [0, i+1, i+2];  // Center, current edge, next edge
}
```

**Segment Examples:**

| Segments | Shape | Vertices | Triangles | Use Case |
|----------|-------|----------|-----------|----------|
| 3 | Triangle | 5 | 3 | Minimal geometry |
| 4 | Square | 6 | 4 | Quad placeholder |
| 6 | Hexagon | 8 | 6 | Low-poly aesthetic |
| 8 | Octagon | 10 | 8 | Rounded corners |
| 16 | Smooth circle | 18 | 16 | Default quality |
| 32 | Very smooth | 34 | 32 | High quality (default) |
| 64 | Ultra smooth | 66 | 64 | Close-up viewing |
| 128 | Perfect circle | 130 | 128 | Maximum quality |

**Example Usage:**

```cpp
// Default quality circle (32 segments)
auto circle = Mesh::CreateCircle();

// Low-poly hexagon
auto hexagon = Mesh::CreateCircle(6);

// High-quality smooth circle
auto smoothCircle = Mesh::CreateCircle(64);

// Create entity with circle mesh
Entity disc = SceneAPI::CreateMeshEntity(*scene, "Disc", 
                                        circle, 
                                        glm::vec3(2, 0, 0));
```

**Common Use Cases:**
- Ground planes/platforms
- Circular UI elements
- Wheels and discs
- Target indicators
- Area markers
- Low-poly terrain

**Performance Notes:**
- More segments = smoother appearance but more GPU cost
- Use 8-16 segments for distant objects
- Use 32-64 segments for close-up objects
- Avoid exceeding 128 segments (diminishing returns)

---

### Accessors

```cpp
VertexArray* GetVertexArray() const;
uint32_t GetIndexCount() const;
PrimitiveType GetType() const;
```

**GetVertexArray()**
- **Returns:** Raw pointer to the internal VAO
- **Usage:** Bind before rendering
- **Lifetime:** Valid as long as the Mesh exists

**GetIndexCount()**
- **Returns:** Number of indices (for `glDrawElements`)
- **Always a multiple of 3** (triangle primitives)

**GetType()**
- **Returns:** `PrimitiveType` enum value
- **Usage:** Identify mesh type for debugging or special handling

**Example:**
```cpp
auto mesh = Mesh::CreateCube();

// Manual rendering
mesh->GetVertexArray()->Bind();
glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_INT, nullptr);

// Query mesh info
std::cout << "Mesh has " << mesh->GetIndexCount() / 3 << " triangles\n";
```

---

## Buffer System

**Location:** `Engine/Rendering/Buffers/`

### Vertex Buffer

**File:** `Buffer.hpp/cpp`

Wraps OpenGL Vertex Buffer Object (VBO).

```cpp
class VertexBuffer {
public:
    VertexBuffer(const void* data, uint32_t size);
    ~VertexBuffer();
    
    void Bind() const;
    void Unbind() const;
};
```

**Example:**
```cpp
float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

auto vb = new VertexBuffer(vertices, sizeof(vertices));
```

### Index Buffer

**File:** `Buffer.hpp/cpp`

Wraps OpenGL Element Buffer Object (EBO).

```cpp
class IndexBuffer {
public:
    IndexBuffer(const uint32_t* indices, uint32_t count);
    ~IndexBuffer();
    
    void Bind() const;
    void Unbind() const;
    uint32_t GetCount() const;
};
```

**Example:**
```cpp
uint32_t indices[] = { 0, 1, 2 };
auto ib = new IndexBuffer(indices, 3);
```

### Vertex Array

**File:** `VertexArray.hpp/cpp`

Wraps OpenGL Vertex Array Object (VAO).

```cpp
class VertexArray {
public:
    VertexArray();
    ~VertexArray();  // Deletes owned VB and IB
    
    void Bind() const;
    void Unbind() const;
    
    void AddVertexBuffer(VertexBuffer* vb);
    void SetIndexBuffer(IndexBuffer* ib);
    
    IndexBuffer* GetIndexBuffer() const;
};
```

**Example:**
```cpp
auto va = new VertexArray();

auto vb = new VertexBuffer(vertices, sizeof(vertices));
auto ib = new IndexBuffer(indices, 3);

va->AddVertexBuffer(vb);
va->SetIndexBuffer(ib);

// Ownership transferred to VAO
// VAO destructor will delete vb and ib
```

**Vertex Attributes:**
Currently hardcoded in `AddVertexBuffer()`:
```cpp
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
```

**Future:** Flexible vertex layout system with stride/offset calculation.

---

## Shader System

**Location:** `Engine/Rendering/Shaders/Shader.hpp/cpp`

### Shader Class

Manages OpenGL shader programs.

```cpp
class Shader {
public:
    Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
    ~Shader();
    
    void Bind() const;
    void Unbind() const;
    
    void SetInt(const std::string& name, int value);
    void SetFloat(const std::string& name, float value);
    void SetVec3(const std::string& name, const glm::vec3& value);
    void SetMat4(const std::string& name, const glm::mat4& value);
};
```

### Creating Shaders

```cpp
std::string vertexSrc = R"(
    #version 460 core
    layout(location = 0) in vec3 aPos;
    
    uniform mat4 u_ViewProj;
    uniform mat4 u_Model;
    
    void main() {
        gl_Position = u_ViewProj * u_Model * vec4(aPos, 1.0);
    }
)";

std::string fragmentSrc = R"(
    #version 460 core
    out vec4 FragColor;
    
    void main() {
        FragColor = vec4(1.0, 0.5, 0.2, 1.0);
    }
)";

auto shader = std::make_unique<Shader>(vertexSrc, fragmentSrc);
```

### Setting Uniforms

```cpp
shader->Bind();
shader->SetMat4("u_ViewProj", camera.GetViewProjection());
shader->SetMat4("u_Model", transform);
shader->SetVec3("u_Color", glm::vec3(1.0f, 0.5f, 0.2f));
shader->SetFloat("u_Time", glfwGetTime());
```

**Uniform Lookup:**
Uses `glGetUniformLocation()` on every set call.

**Future Optimization:** Cache uniform locations in a map.

---

## Framebuffer System

**Location:** `Engine/Rendering/Framebuffer/Framebuffer.hpp/cpp`

### Purpose

Offscreen render target for rendering scene to a texture, then displaying in ImGui viewport.

### Framebuffer Class

```cpp
class Framebuffer {
public:
    Framebuffer(uint32_t width, uint32_t height);
    ~Framebuffer();
    
    void Bind() const;
    void Unbind() const;
    void Resize(uint32_t width, uint32_t height);
    
    uint32_t GetColorAttachmentID() const;
};
```

### Creating Framebuffer

```cpp
auto framebuffer = std::make_unique<Framebuffer>(1280, 720);
```

**What It Creates:**
1. **Framebuffer Object (FBO)**
2. **Color Attachment** - RGBA8 texture
3. **Depth Attachment** - Depth24Stencil8 renderbuffer

### Rendering to Framebuffer

```cpp
// Bind framebuffer
framebuffer->Bind();

// Render scene
Renderer::Clear({0.1f, 0.1f, 0.1f, 1.0f});
Renderer::BeginScene(camera.GetViewProjection());
// ... submit meshes ...
Renderer::EndScene();

// Unbind (render to screen)
framebuffer->Unbind();
```

### Displaying in ImGui

```cpp
ImGui::Begin("Viewport");

uint32_t textureID = framebuffer->GetColorAttachmentID();
ImVec2 viewportSize = ImGui::GetContentRegionAvail();

ImGui::Image(
    (void*)(intptr_t)textureID,
    viewportSize,
    ImVec2(0, 1),  // UV top-left (flip Y)
    ImVec2(1, 0)   // UV bottom-right
);

ImGui::End();
```

**UV Flip:** OpenGL textures have origin at bottom-left, ImGui expects top-left.

### Resizing

```cpp
void Framebuffer::Resize(uint32_t width, uint32_t height);
```

Deletes and recreates attachments. Called when viewport panel resizes.

**Example:**
```cpp
ImVec2 viewportSize = ImGui::GetContentRegionAvail();
if (viewportSize.x != lastWidth || viewportSize.y != lastHeight) {
    framebuffer->Resize((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);
    lastWidth = viewportSize.x;
    lastHeight = viewportSize.y;
}
```

---

## Camera System

**Location:** `Engine/Rendering/Camera/EditorCamera.hpp/cpp`

### EditorCamera Class

Free-flying camera with WASD movement and mouse look.

```cpp
class EditorCamera {
public:
    EditorCamera(float fov = 45.0f, float aspectRatio = 16.0f/9.0f,
                 float nearClip = 0.1f, float farClip = 1000.0f);
    
    void ProcessKeyboard(const glm::vec3& direction, float deltaTime);
    void ProcessMouseMovement(float xOffset, float yOffset);
    
    const glm::mat4& GetViewMatrix() const;
    const glm::mat4& GetProjectionMatrix() const;
    glm::mat4 GetViewProjection() const;
    
    void SetAspectRatio(float aspectRatio);
};
```

### Movement Processing

```cpp
void ProcessKeyboard(const glm::vec3& direction, float deltaTime);
```

**Parameters:**
- `direction` - Movement vector in camera-local space:
  - `x` - Right/Left
  - `y` - Up/Down
  - `z` - Forward/Backward
- `deltaTime` - Frame delta time for framerate-independent movement

**Example:**
```cpp
glm::vec3 dir{0.0f};
if (Input::IsKeyPressed(GLFW_KEY_W)) dir.z += 1.0f;  // Forward
if (Input::IsKeyPressed(GLFW_KEY_S)) dir.z -= 1.0f;  // Backward
if (Input::IsKeyPressed(GLFW_KEY_A)) dir.x -= 1.0f;  // Left
if (Input::IsKeyPressed(GLFW_KEY_D)) dir.x += 1.0f;  // Right
if (Input::IsKeyPressed(GLFW_KEY_E)) dir.y += 1.0f;  // Up
if (Input::IsKeyPressed(GLFW_KEY_Q)) dir.y -= 1.0f;  // Down

camera.ProcessKeyboard(dir, deltaTime);
```

### Mouse Look

```cpp
void ProcessMouseMovement(float xOffset, float yOffset);
```

**Parameters:**
- `xOffset` - Horizontal mouse delta (positive = right)
- `yOffset` - Vertical mouse delta (positive = up)

**Example:**
```cpp
double dx, dy;
ViewportInput::GetMouseDelta(dx, dy);
camera.ProcessMouseMovement((float)dx, (float)dy);
```

**Constraints:**
- Pitch clamped to ±89° to prevent gimbal lock

### Matrix Retrieval

```cpp
glm::mat4 viewProj = camera.GetViewProjection();
Renderer::BeginScene(viewProj);
```

**View Matrix:** Transforms world space → camera space  
**Projection Matrix:** Transforms camera space → clip space  
**View-Projection:** Combined transformation

### Aspect Ratio Updates

```cpp
void SetAspectRatio(float aspectRatio);
```

Called when viewport resizes:
```cpp
float aspect = viewportWidth / viewportHeight;
camera.SetAspectRatio(aspect);
```

---

## Complete Rendering Example

### Setup (EditorLayer::OnAttach)

```cpp
// Create framebuffer
m_Framebuffer = std::make_unique<Framebuffer>(1280, 720);

// Create shader
std::string vs = R"(
    #version 460 core
    layout(location = 0) in vec3 aPos;
    uniform mat4 u_ViewProj;
    uniform mat4 u_Model;
    void main() {
        gl_Position = u_ViewProj * u_Model * vec4(aPos, 1.0);
    }
)";

std::string fs = R"(
    #version 460 core
    out vec4 FragColor;
    void main() {
        FragColor = vec4(0.8, 0.5, 0.2, 1.0);
    }
)";

m_Shader = std::make_unique<Shader>(vs, fs);

// Create scene with meshes
SceneAPI::CreateDefaultScene(*m_ActiveScene);
```

### Render Loop (EditorLayer::OnImGuiRender)

```cpp
// Render to framebuffer
m_Framebuffer->Bind();
Renderer::Clear({0.12f, 0.12f, 0.14f, 1.0f});

// Begin scene with camera
Renderer::BeginScene(m_EditorCamera.GetViewProjection());

// Query all entities with mesh
auto& reg = m_ActiveScene->Reg();
auto view = reg.view<TransformComponent, MeshComponent>();

for (auto entity : view) {
    auto& tc = view.get<TransformComponent>(entity);
    auto& mc = view.get<MeshComponent>(entity);
    
    if (mc.MeshHandle) {
        Renderer::Submit(mc.MeshHandle, tc.GetMatrix(), *m_Shader);
    }
}

Renderer::EndScene();
m_Framebuffer->Unbind();

// Display in ImGui viewport
ImGui::Begin("Viewport");
ImGui::Image((void*)(intptr_t)m_Framebuffer->GetColorAttachmentID(),
             ImGui::GetContentRegionAvail(),
             ImVec2(0, 1), ImVec2(1, 0));
ImGui::End();
```

---

## Future Enhancements

### Batching

Group meshes by shader and material to reduce state changes:
```cpp
Renderer::Submit(meshes, transforms, shader);  // Batch submit
```

### Material System

Encapsulate shader + textures + parameters:
```cpp
struct Material {
    std::shared_ptr<Shader> Shader;
    std::shared_ptr<Texture> AlbedoMap;
    glm::vec3 Color;
    float Metallic;
    float Roughness;
};
```

### Lighting

Add light components and deferred rendering pipeline:
```cpp
struct PointLightComponent {
    glm::vec3 Color;
    float Intensity;
    float Radius;
};
```

### Shadows

Shadow mapping with cascaded shadow maps for directional lights.

### Post-Processing

Framebuffer chain for effects (bloom, tone mapping, FXAA).

---

## Related Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md#rendering-subsystem) - System overview
- [SCENE_SYSTEM.md](SCENE_SYSTEM.md) - ECS and entity rendering
- [INPUT_SYSTEM.md](INPUT_SYSTEM.md) - Camera controls
