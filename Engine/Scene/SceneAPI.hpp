#pragma once

#include <string>
#include <memory>

#include "Scene.hpp"
#include "Entity.hpp"
#include "Components.hpp"
#include <Rendering/Mesh/Mesh.hpp>
#include <glm/glm.hpp>
#include <Core/UUID.hpp>

namespace SceneAPI
{
    inline void SetNextOrder(Entity entity)
    {
        auto& reg = entity.GetScene()->Reg();
        int32_t maxOrder = -1;
        reg.view<HierarchyOrderComponent>().each([&](auto e, auto& hc) {
            if (hc.Order > maxOrder) maxOrder = hc.Order;
        });
        entity.AddOrReplaceComponent<HierarchyOrderComponent>(maxOrder + 1);
    }

    inline Entity CreateEmptyEntity(Scene& scene, const std::string& name = "Empty Entity")
    {
        Entity entity = scene.CreateEntity(name);
        entity.AddComponent<TagComponent>(name);
        entity.AddComponent<TransformComponent>();
        SetNextOrder(entity);
        return entity;
    }

    inline Entity CreateCameraEntity(Scene& scene, const std::string& name = "Camera")
    {
        Entity camera = scene.CreateEntity(name);
        camera.AddComponent<TagComponent>(name);
        camera.AddComponent<TransformComponent>();
        camera.AddComponent<CameraComponent>();
        SetNextOrder(camera);
        return camera;
    }

    inline Entity CreateMeshEntity(Scene& scene,
                                   const std::string& name,
                                   const std::shared_ptr<Mesh>& mesh,
                                   const glm::vec3& position = { 0.0f, 0.0f, 0.0f })
    {
        Entity entity = scene.CreateEntity(name);
        entity.AddComponent<TagComponent>(name);
        entity.AddComponent<TransformComponent>(position);
        entity.AddComponent<MeshComponent>(mesh);
        SetNextOrder(entity);
        return entity;
    }

    inline Entity CreateMeshEntityWithUUID(Scene& scene,
                                   Core::UUID uuid,
                                   const std::string& name,
                                   const std::shared_ptr<Mesh>& mesh,
                                   const glm::vec3& position = { 0.0f, 0.0f, 0.0f })
    {
        Entity entity = scene.CreateEntityWithUUID(uuid, name);
        entity.AddComponent<TagComponent>(name);
        entity.AddComponent<TransformComponent>(position);
        entity.AddComponent<MeshComponent>(mesh);
        SetNextOrder(entity);
        return entity;
    }

    inline Entity DuplicateEntity(Scene& scene, Entity source, bool isLinked = false)
    {
        if (!source) return Entity();

        std::string name = "Entity";
        if (source.HasComponent<TagComponent>())
            name = source.GetComponent<TagComponent>().Tag;

        // Simple name indexing logic: if ends with number, increment, else add " 2"
        // For simplicity here just add " 2" or " (Instance)"
        name += isLinked ? " (Instance)" : " 2";

        Entity duplicate = scene.CreateEntity(name);
        duplicate.AddComponent<TagComponent>(name);

        if (source.HasComponent<TransformComponent>())
            duplicate.AddComponent<TransformComponent>(source.GetComponent<TransformComponent>());

        if (source.HasComponent<MeshComponent>())
            duplicate.AddComponent<MeshComponent>(source.GetComponent<MeshComponent>());

        if (source.HasComponent<CameraComponent>())
            duplicate.AddComponent<CameraComponent>();

        if (isLinked)
        {
            auto sourceID = source.GetComponent<IDComponent>().ID;
            duplicate.AddComponent<DuplicationComponent>(sourceID);
        }

        SetNextOrder(duplicate);
        return duplicate;
    }

    inline void CreateDefaultScene(Scene& scene)
    {
        CreateCameraEntity(scene);
        CreateMeshEntity(scene, "Cube",     Mesh::CreateCube(),     { -0.4f, 0.5f, 0.0f });
        CreateMeshEntity(scene, "Triangle", Mesh::CreateTriangle3D(), { -1.5f, 0.3f, 0.0f });
        CreateMeshEntity(scene, "Circle",   Mesh::CreateCircle(32), { 1.5f, 0.0f, 0.0f });
    }
}
