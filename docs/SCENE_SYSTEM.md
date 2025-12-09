# Scene System & ECS

Documentation for the Entity-Component-System (ECS) architecture using EnTT and the scene management system.

---

## Overview

The UI Engine uses an Entity-Component-System (ECS) architecture for scene management, powered by the high-performance [EnTT](https://github.com/skypjack/entt) library.

### What is ECS?

**Traditional Object-Oriented:**
```cpp
class GameObject {
    Transform transform;
    Mesh mesh;
    RigidBody physics;
    
    virtual void Update();
};
```

**ECS:**
```cpp
// Entities are just IDs
Entity entity = scene.CreateEntity();

// Components are pure data
entity.AddComponent<TransformComponent>(position);
entity.AddComponent<MeshComponent>(mesh);
entity.AddComponent<RigidBodyComponent>(mass);

// Systems process components
PhysicsSystem::Update(registry.view<TransformComponent, RigidBodyComponent>());
```

### Benefits

- **Performance:** Cache-friendly, data-oriented design
- **Flexibility:** Add/remove components at runtime
- **Decoupling:** Components don't depend on each other
- **Composition:** Mix and match components as needed

---

## Scene Class

**Location:** `Engine/Scene/Scene.hpp/cpp`

### Scene Responsibilities

- Owns the EnTT registry
- Manages entity lifecycle (creation, destruction)
- Provides access to registry for queries

### API

#### Creating a Scene

```cpp
auto scene = std::make_unique<Scene>();
```

#### Creating Entities

```cpp
Entity Scene::CreateEntity(const std::string& name = "");
```

**Returns:** `Entity` wrapper around `entt::entity` ID

**Example:**
```cpp
Entity player = scene->CreateEntity("Player");
Entity enemy = scene->CreateEntity("Enemy");
```

#### Accessing Registry

```cpp
entt::registry& Scene::Reg();
```

**Usage:** Query entities and components

**Example:**
```cpp
auto& registry = scene->Reg();
auto view = registry.view<TransformComponent, MeshComponent>();

for (auto entity : view) {
    auto& transform = view.get<TransformComponent>(entity);
    auto& mesh = view.get<MeshComponent>(entity);
    // ...
}
```

---

## Entity Class

**Location:** `Engine/Scene/Entity.hpp`

### Entity Wrapper

Lightweight wrapper around `entt::entity` handle providing a convenient API.

```cpp
class Entity {
public:
    Entity(entt::entity handle, Scene* scene);
    
    template<typename T, typename... Args>
    T& AddComponent(Args&&... args);
    
    template<typename T>
    T& GetComponent();
    
    template<typename T>
    bool HasComponent();
    
    template<typename T>
    void RemoveComponent();
    
    operator bool() const;  // Valid entity check
    operator uint32_t() const;  // Get ID
};
```

### Creating Entities

```cpp
Entity scene::CreateEntity(const std::string& name) {
    Entity entity = { m_Registry.create(), this };
    entity.AddComponent<TagComponent>(name);
    return entity;
}
```

### Component Management

#### Adding Components

```cpp
template<typename T, typename... Args>
T& Entity::AddComponent(Args&&... args);
```

**Example:**
```cpp
Entity cube = scene->CreateEntity("Cube");

// Add transform at origin
auto& transform = cube.AddComponent<TransformComponent>();

// Add transform at specific position
auto& transform2 = cube.AddComponent<TransformComponent>(glm::vec3(0, 5, 0));

// Add mesh
cube.AddComponent<MeshComponent>(Mesh::CreateCube());
```

#### Getting Components

```cpp
template<typename T>
T& Entity::GetComponent();
```

**Example:**
```cpp
auto& transform = cube.GetComponent<TransformComponent>();
transform.Position.y += 1.0f;
```

**Note:** Throws assertion if component doesn't exist. Check with `HasComponent()` first.

#### Checking for Components

```cpp
template<typename T>
bool Entity::HasComponent();
```

**Example:**
```cpp
if (entity.HasComponent<MeshComponent>()) {
    auto& mesh = entity.GetComponent<MeshComponent>();
    // Render mesh
}
```

#### Removing Components

```cpp
template<typename T>
void Entity::RemoveComponent();
```

**Example:**
```cpp
entity.RemoveComponent<MeshComponent>();  // Stop rendering this entity
```

---

## Components

**Location:** `Engine/Scene/Components.hpp`

All components are plain data structures (POD or near-POD).

### TagComponent

Human-readable entity name.

```cpp
struct TagComponent {
    std::string Tag;
    
    TagComponent() = default;
    TagComponent(const std::string& tag) : Tag(tag) {}
};
```

**Usage:**
```cpp
entity.AddComponent<TagComponent>("Player");

auto& tag = entity.GetComponent<TagComponent>();
std::cout << "Entity name: " << tag.Tag << std::endl;
```

### TransformComponent

3D spatial transformation (position, rotation, scale).

```cpp
struct TransformComponent {
    glm::vec3 Position{0.0f, 0.0f, 0.0f};
    glm::vec3 Rotation{0.0f, 0.0f, 0.0f};  // Euler angles (degrees)
    glm::vec3 Scale{1.0f, 1.0f, 1.0f};
    
    TransformComponent() = default;
    TransformComponent(const glm::vec3& pos) : Position(pos) {}
    
    glm::mat4 GetMatrix() const;
};
```

**Matrix Calculation:**
```cpp
glm::mat4 TransformComponent::GetMatrix() const {
    // Translation
    glm::mat4 T = glm::translate(glm::mat4(1.0f), Position);
    
    // Rotation (Euler angles)
    glm::mat4 R = glm::mat4(1.0f);
    R = glm::rotate(R, glm::radians(Rotation.x), glm::vec3(1, 0, 0));  // Pitch
    R = glm::rotate(R, glm::radians(Rotation.y), glm::vec3(0, 1, 0));  // Yaw
    R = glm::rotate(R, glm::radians(Rotation.z), glm::vec3(0, 0, 1));  // Roll
    
    // Scale
    glm::mat4 S = glm::scale(glm::mat4(1.0f), Scale);
    
    // Combine: T * R * S
    return T * R * S;
}
```

**Usage:**
```cpp
auto& transform = entity.GetComponent<TransformComponent>();

// Move entity
transform.Position += glm::vec3(0, 1, 0);

// Rotate entity (45° around Y-axis)
transform.Rotation.y += 45.0f;

// Scale entity
transform.Scale *= 2.0f;

// Get model matrix for rendering
glm::mat4 modelMatrix = transform.GetMatrix();
```

### MeshComponent

Reference to a renderable mesh.

```cpp
struct MeshComponent {
    std::shared_ptr<Mesh> MeshHandle;
    
    MeshComponent() = default;
    MeshComponent(const std::shared_ptr<Mesh>& mesh) : MeshHandle(mesh) {}
};
```

**Usage:**
```cpp
auto mesh = Mesh::CreateCube();
entity.AddComponent<MeshComponent>(mesh);

// Later, during rendering:
auto& mc = entity.GetComponent<MeshComponent>();
if (mc.MeshHandle) {
    Renderer::Submit(mc.MeshHandle, transform, shader);
}
```

**Memory Management:**
- `std::shared_ptr` allows multiple entities to reference the same mesh
- Mesh automatically deleted when last reference is destroyed

---

## SceneAPI

**Location:** `Engine/Scene/SceneAPI.hpp`

Helper functions for common scene operations.

### Create Camera Entity

```cpp
Entity SceneAPI::CreateCamera(Scene& scene, const std::string& name = "Camera");
```

**What It Does:**
- Creates entity with `TagComponent` and `TransformComponent`
- Returns entity for further configuration

**Example:**
```cpp
Entity camera = SceneAPI::CreateCamera(*scene, "Main Camera");
camera.GetComponent<TransformComponent>().Position = glm::vec3(0, 10, 20);
```

### Create Mesh Entity

```cpp
Entity SceneAPI::CreateMeshEntity(Scene& scene,
                                  const std::string& name,
                                  const std::shared_ptr<Mesh>& mesh,
                                  const glm::vec3& position = {0, 0, 0});
```

**What It Does:**
- Creates entity with `TagComponent`, `TransformComponent`, and `MeshComponent`
- Sets initial position
- Returns entity

**Example:**
```cpp
auto cubeMesh = Mesh::CreateCube();
Entity cube = SceneAPI::CreateMeshEntity(*scene, "Cube", cubeMesh, glm::vec3(0, 1, 0));

// Further configure
cube.GetComponent<TransformComponent>().Scale = glm::vec3(2.0f);
```

### Create Default Scene

```cpp
void SceneAPI::CreateDefaultScene(Scene& scene);
```

**What It Does:**
Populates scene with starting content:
- Camera entity
- Cube at (0, 0.5, 0)
- Triangle at (-1.5, 0.3, 0)
- Circle at (1.5, 0, 0)

**Example:**
```cpp
auto scene = std::make_unique<Scene>();
SceneAPI::CreateDefaultScene(*scene);
```

---

## Querying Components (EnTT Views)

EnTT provides efficient component queries using **views**.

### Single Component Query

```cpp
auto& registry = scene->Reg();
auto view = registry.view<TransformComponent>();

for (auto entity : view) {
    auto& transform = view.get<TransformComponent>(entity);
    // Update all transforms
}
```

### Multi-Component Query

```cpp
auto view = registry.view<TransformComponent, MeshComponent>();

for (auto entity : view) {
    auto& transform = view.get<TransformComponent>(entity);
    auto& mesh = view.get<MeshComponent>(entity);
    
    // Only entities with BOTH components
    Renderer::Submit(mesh.MeshHandle, transform.GetMatrix(), shader);
}
```

### Excluding Components

```cpp
auto view = registry.view<TransformComponent>(entt::exclude<MeshComponent>);

for (auto entity : view) {
    // Entities with Transform but WITHOUT Mesh
    // E.g., cameras, lights, collision volumes
}
```

### Iterating with Multiple Components

```cpp
auto view = registry.view<TransformComponent, MeshComponent>();

for (auto [entity, transform, mesh] : view.each()) {
    // Structured bindings (C++17)
    transform.Position.y += 0.1f;
    Renderer::Submit(mesh.MeshHandle, transform.GetMatrix(), shader);
}
```

---

## Complete Scene Example

### Scene Setup

```cpp
#include <Scene/Scene.hpp>
#include <Scene/SceneAPI.hpp>
#include <Scene/Components.hpp>

// Create scene
auto scene = std::make_unique<Scene>();

// Create entities manually
Entity player = scene->CreateEntity("Player");
player.AddComponent<TransformComponent>(glm::vec3(0, 1, 0));
player.AddComponent<MeshComponent>(Mesh::CreateCube());

// Or use SceneAPI
auto enemyMesh = Mesh::CreateCircle(8);  // Octagon
Entity enemy = SceneAPI::CreateMeshEntity(*scene, "Enemy", enemyMesh, glm::vec3(5, 0, 0));

// Configure enemy
auto& enemyTransform = enemy.GetComponent<TransformComponent>();
enemyTransform.Scale = glm::vec3(0.5f);
enemyTransform.Rotation.y = 45.0f;
```

### Update Loop

```cpp
void UpdateScene(Scene& scene, float deltaTime) {
    auto& registry = scene.Reg();
    
    // Animate all entities
    auto view = registry.view<TransformComponent>();
    for (auto entity : view) {
        auto& transform = view.get<TransformComponent>(entity);
        transform.Rotation.y += 30.0f * deltaTime;  // 30°/sec
    }
}
```

### Render Loop

```cpp
void RenderScene(Scene& scene, Shader& shader, const glm::mat4& viewProj) {
    Renderer::BeginScene(viewProj);
    
    auto& registry = scene.Reg();
    auto view = registry.view<TransformComponent, MeshComponent>();
    
    for (auto entity : view) {
        auto& transform = view.get<TransformComponent>(entity);
        auto& mesh = view.get<MeshComponent>(entity);
        
        if (mesh.MeshHandle) {
            Renderer::Submit(mesh.MeshHandle, transform.GetMatrix(), shader);
        }
    }
    
    Renderer::EndScene();
}
```

---

## Advanced EnTT Features

### Listeners (Future)

React to component changes:
```cpp
registry.on_construct<TransformComponent>().connect<&OnTransformAdded>();
registry.on_destroy<MeshComponent>().connect<&OnMeshRemoved>();
```

### Groups (Future)

Pre-sorted component groups for faster iteration:
```cpp
auto group = registry.group<TransformComponent, MeshComponent>();
```

### Signals (Future)

Custom events:
```cpp
entt::dispatcher dispatcher;
dispatcher.trigger(EntitySpawnedEvent{entity});
```

### Snapshots (Future)

Save/load entire scene state:
```cpp
entt::snapshot{registry}.entities(output).component<TransformComponent, MeshComponent>(output);
```

---

## Performance Considerations

### Cache-Friendly Iteration

EnTT stores components in contiguous arrays (sparse sets). Iterating views is cache-efficient.

### Avoid Component Bloat

Keep components small and focused:
```cpp
// GOOD: Separate components
struct TransformComponent { /* 48 bytes */ };
struct RenderComponent { /* 32 bytes */ };

// BAD: Monolithic component
struct GameObject { /* 200+ bytes */ };
```

### Component Pooling

EnTT reuses entity IDs and component storage automatically.

### Query Optimization

Cache views for repeated queries:
```cpp
// SLOW: Create view every frame
for (int i = 0; i < 1000; i++) {
    auto view = registry.view<TransformComponent>();
    // ...
}

// FAST: Reuse view
auto view = registry.view<TransformComponent>();
for (int i = 0; i < 1000; i++) {
    // ...
}
```

---

## Future Component Ideas

### Physics Component

```cpp
struct RigidBodyComponent {
    glm::vec3 Velocity{0.0f};
    glm::vec3 AngularVelocity{0.0f};
    float Mass = 1.0f;
    bool IsKinematic = false;
};
```

### Lighting Components

```cpp
struct PointLightComponent {
    glm::vec3 Color{1.0f, 1.0f, 1.0f};
    float Intensity = 1.0f;
    float Radius = 10.0f;
};

struct DirectionalLightComponent {
    glm::vec3 Direction{0.0f, -1.0f, 0.0f};
    glm::vec3 Color{1.0f, 1.0f, 1.0f};
    float Intensity = 1.0f;
};
```

### Hierarchy Component

```cpp
struct HierarchyComponent {
    Entity Parent;
    std::vector<Entity> Children;
};
```

### Material Component

```cpp
struct MaterialComponent {
    glm::vec3 Albedo{1.0f};
    float Metallic = 0.0f;
    float Roughness = 0.5f;
    std::shared_ptr<Texture> AlbedoMap;
    std::shared_ptr<Texture> NormalMap;
};
```

---

## Comparison: Traditional vs ECS

### Adding a Feature (e.g., Physics)

**Traditional OOP:**
```cpp
class GameObject {
    virtual void Update() {
        // Every object pays virtual call cost
        if (hasPhysics) {
            // Only some objects need this
        }
    }
};
```

**ECS:**
```cpp
// Only entities with RigidBodyComponent are processed
auto view = registry.view<TransformComponent, RigidBodyComponent>();
for (auto [entity, transform, rb] : view.each()) {
    // Physics simulation
    transform.Position += rb.Velocity * dt;
}
```

### Memory Layout

**OOP:** Objects scattered in memory (cache misses)  
**ECS:** Components packed in arrays (cache hits)

```
OOP:  [Obj1][....][Obj2][......][Obj3]
ECS:  [T1][T2][T3]...[M1][M2][M3]...
      (Transforms)     (Meshes)
```

---

## Debugging Tips

### Print All Entities

```cpp
auto view = registry.view<TagComponent>();
for (auto entity : view) {
    auto& tag = view.get<TagComponent>(entity);
    std::cout << "Entity " << (uint32_t)entity << ": " << tag.Tag << std::endl;
}
```

### Count Components

```cpp
size_t transformCount = registry.view<TransformComponent>().size();
size_t meshCount = registry.view<MeshComponent>().size();
std::cout << "Transforms: " << transformCount << ", Meshes: " << meshCount << std::endl;
```

### Validate Entity

```cpp
if (registry.valid(entityHandle)) {
    // Entity exists
} else {
    // Entity was destroyed
}
```

---

## Related Documentation

- [ARCHITECTURE.md](ARCHITECTURE.md#scene-subsystem) - Scene system overview
- [RENDERING.md](RENDERING.md) - Rendering with ECS
- [EnTT Documentation](https://github.com/skypjack/entt/wiki) - Official EnTT guide
