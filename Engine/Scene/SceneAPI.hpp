#pragma once

#include <string>
#include <memory>

#include "Scene.hpp"
#include "Entity.hpp"
#include "Components.hpp"
#include <Rendering/Mesh/Mesh.hpp>
#include <glm/glm.hpp>

namespace SceneAPI
{
    inline Entity CreateCamera(Scene& scene, const std::string& name = "Camera")
    {
        Entity camera = scene.CreateEntity(name);
        camera.AddComponent<TagComponent>(name);
        camera.AddComponent<TransformComponent>();
        return camera;
    }

    inline Entity CreateMeshEntity(Scene& scene,
                                   const std::string& name,
                                   const std::shared_ptr<Mesh>& mesh,
                                   const glm::vec3& position = { 0.0f, 0.0f, 0.0f })
    {
        Entity entity = scene.CreateEntity(name);
        entity.AddComponent<TagComponent>(name);
        auto& tc = entity.AddComponent<TransformComponent>(position);
        (void)tc;
        entity.AddComponent<MeshComponent>(mesh);
        return entity;
    }

    inline void CreateDefaultScene(Scene& scene)
    {
        // Camera
        CreateCamera(scene);

        // For now: three default meshes (you can change this to only one "Plane" later)
        CreateMeshEntity(scene, "Cube",     Mesh::CreateCube(),     { 0.0f, 0.5f, 0.0f });
        CreateMeshEntity(scene, "Cube2",    Mesh::CreateCube(),     { 1.5f, 0.5f, 0.0f });
        CreateMeshEntity(scene, "Cube3",    Mesh::CreateCube(),     { 3.0f, -0.5f, 0.0f });
        // CreateMeshEntity(scene, "Triangle", Mesh::CreateTriangle3D(), { -1.5f, 0.3f, 0.0f });
        // CreateMeshEntity(scene, "Circle",   Mesh::CreateCircle(32), { 1.5f, 0.0f, 0.0f });
    }
}
