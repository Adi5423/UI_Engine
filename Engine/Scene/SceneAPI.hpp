#pragma once

#include <string>
#include <memory>
#include <cctype>

#include "Scene.hpp"
#include "Entity.hpp"
#include "Components.hpp"
#include <Rendering/Mesh/Mesh.hpp>
#include <glm/glm.hpp>
#include <Core/UUID.hpp>

namespace SceneAPI
{
    // =========================================================================
    // Helper: strip a trailing " (N)" suffix from a name.
    // Returns the base name without the suffix.
    // e.g. "Cube (3)" → "Cube",  "Cube" → "Cube"
    // =========================================================================
    inline std::string StripNumericSuffix(const std::string& name)
    {
        if (name.empty() || name.back() != ')')
            return name;

        // Walk backwards: find matching '('
        size_t closePos = name.size() - 1;
        size_t openPos  = name.rfind('(');
        if (openPos == std::string::npos || openPos == 0)
            return name;

        // Everything between '(' and ')' must be digits
        std::string inner = name.substr(openPos + 1, closePos - openPos - 1);
        if (inner.empty())
            return name;
        for (char c : inner)
            if (!std::isdigit(static_cast<unsigned char>(c)))
                return name;

        // There must be a space before '('
        size_t spacePos = openPos - 1;
        if (name[spacePos] != ' ')
            return name;

        return name.substr(0, spacePos);
    }

    // =========================================================================
    // NAMING RULE:
    //   - First entity with a given base name keeps the bare name (e.g. "Cube")
    //   - Second gets "(1)"  → "Cube (1)"
    //   - Third gets  "(2)"  → "Cube (2)"
    //   ... and so on.
    // The function strips any existing "(N)" suffix before searching so that
    // duplicating "Cube (1)" still competes in the "Cube" namespace.
    // =========================================================================
    inline std::string GenerateUniqueName(Scene& scene, const std::string& desiredName)
    {
        // Strip existing "(N)" suffix to obtain the base name.
        std::string baseName = StripNumericSuffix(desiredName);

        // Collect all current tags in the scene.
        auto& reg = scene.Reg();
        bool bareNameUsed = false;
        int  highestIndex = 0; // highest (N) seen for this base name

        reg.view<TagComponent>().each([&](auto /*e*/, const TagComponent& tc)
        {
            const std::string& tag = tc.Tag;

            // Exact match → bare name is already taken.
            if (tag == baseName)
            {
                bareNameUsed = true;
                return;
            }

            // Match "baseName (N)" pattern.
            std::string prefix = baseName + " (";
            if (tag.size() > prefix.size() &&
                tag.substr(0, prefix.size()) == prefix &&
                tag.back() == ')')
            {
                std::string numStr = tag.substr(prefix.size(), tag.size() - prefix.size() - 1);
                bool allDigits = !numStr.empty();
                for (char c : numStr)
                    if (!std::isdigit(static_cast<unsigned char>(c))) { allDigits = false; break; }
                if (allDigits)
                {
                    int idx = std::stoi(numStr);
                    if (idx > highestIndex) highestIndex = idx;
                    bareNameUsed = true; // at least one variant exists
                }
            }
        });

        if (!bareNameUsed)
            return baseName;                                    // no collision

        return baseName + " (" + std::to_string(highestIndex + 1) + ")";
    }

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
        std::string uniqueName = GenerateUniqueName(scene, name);
        Entity entity = scene.CreateEntity(uniqueName);
        entity.AddComponent<TagComponent>(uniqueName);
        entity.AddComponent<TransformComponent>();
        SetNextOrder(entity);
        return entity;
    }

    inline Entity CreateCameraEntity(Scene& scene, const std::string& name = "Camera")
    {
        std::string uniqueName = GenerateUniqueName(scene, name);
        Entity camera = scene.CreateEntity(uniqueName);
        camera.AddComponent<TagComponent>(uniqueName);
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
        std::string uniqueName = GenerateUniqueName(scene, name);
        Entity entity = scene.CreateEntity(uniqueName);
        entity.AddComponent<TagComponent>(uniqueName);
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
        std::string uniqueName = GenerateUniqueName(scene, name);
        Entity entity = scene.CreateEntityWithUUID(uuid, uniqueName);
        entity.AddComponent<TagComponent>(uniqueName);
        entity.AddComponent<TransformComponent>(position);
        entity.AddComponent<MeshComponent>(mesh);
        SetNextOrder(entity);
        return entity;
    }

    inline Entity DuplicateEntity(Scene& scene, Entity source, bool isLinked = false)
    {
        if (!source) return Entity();

        std::string srcName = "Entity";
        if (source.HasComponent<TagComponent>())
            srcName = source.GetComponent<TagComponent>().Tag;

        // Strip any existing "(N)" suffix so duplicates compete in the same namespace.
        srcName = StripNumericSuffix(srcName);
        // For linked duplicates append " (Instance)" as the base before uniquing.
        if (isLinked)
            srcName += " (Instance)";

        std::string uniqueName = GenerateUniqueName(scene, srcName);

        Entity duplicate = scene.CreateEntity(uniqueName);
        duplicate.AddComponent<TagComponent>(uniqueName);

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

    // Duplicate an entity but force the duplicated entity to use a caller-supplied UUID.
    // This is critical for deterministic Undo/Redo (a command can recreate the same entity).
    inline Entity DuplicateEntityWithUUID(Scene& scene, Entity source, Core::UUID newUUID, bool isLinked = false)
    {
        if (!source) return Entity();

        // If the UUID is already in use, do not create a second entity with the same ID.
        // (This can happen if Execute() is called twice without an intervening Undo().)
        Entity existing = scene.GetEntityByUUID(newUUID);
        if (existing) return existing;

        std::string srcName = "Entity";
        if (source.HasComponent<TagComponent>())
            srcName = source.GetComponent<TagComponent>().Tag;

        // Strip any existing "(N)" suffix so duplicates compete in the same namespace.
        srcName = StripNumericSuffix(srcName);
        if (isLinked)
            srcName += " (Instance)";

        std::string uniqueName = GenerateUniqueName(scene, srcName);

        Entity duplicate = scene.CreateEntityWithUUID(newUUID, uniqueName);
        duplicate.AddComponent<TagComponent>(uniqueName);

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
