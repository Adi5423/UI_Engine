#pragma once
#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include <Rendering/Mesh/Mesh.hpp>
#include <Core/UUID.hpp>

// -----------------------------
// ID Component
// -----------------------------
struct IDComponent
{
    Core::UUID ID;

    IDComponent() = default;
    IDComponent(const Core::UUID& uuid)
        : ID(uuid) {}
};

// -----------------------------
// Tag Component
// -----------------------------
struct TagComponent
{
    std::string Tag = "Entity";

    TagComponent() = default;
    TagComponent(const std::string& tag)
        : Tag(tag) {}
};

// -----------------------------
// Transform Component
// -----------------------------
struct TransformComponent
{
    glm::vec3 Position{ 0.0f };
    glm::vec3 Rotation{ 0.0f };
    glm::vec3 Scale{ 1.0f };

    TransformComponent() = default;
    TransformComponent(const glm::vec3& pos)
        : Position(pos) {}

    glm::mat4 GetMatrix() const
    {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), Position);
        
        // Use Quaternion for rotation to match standard engine conventions (XYZ)
        // and avoid Euler order confusion during matrix construction.
        glm::mat4 R = glm::toMat4(glm::quat(glm::radians(Rotation)));

        glm::mat4 S = glm::scale(glm::mat4(1.0f), Scale);

        return T * R * S;
    }
};

// -----------------------------
// Mesh Component  (TOP LEVEL)
// -----------------------------
struct MeshComponent
{
    std::shared_ptr<Mesh> MeshHandle;

    MeshComponent() = default;
    MeshComponent(const std::shared_ptr<Mesh>& mesh)
        : MeshHandle(mesh) {}
};

// -----------------------------
// Hierarchy Order Component
// -----------------------------
struct HierarchyOrderComponent
{
    int32_t Order = 0;

    HierarchyOrderComponent() = default;
    HierarchyOrderComponent(int32_t order) : Order(order) {}
};

// -----------------------------
// Camera Component
// -----------------------------
struct CameraComponent
{
    // For now simple flag or parameters
    float FOV = 45.0f;
    float Near = 0.1f;
    float Far = 1000.0f;

    CameraComponent() = default;
};

// -----------------------------
// Duplication Component
// -----------------------------
struct DuplicationComponent
{
    Core::UUID SourceID;
    
    // Cached source values to calculate diff
    glm::vec3 LastSourcePosition{ 0.0f };
    glm::vec3 LastSourceRotation{ 0.0f };
    glm::vec3 LastSourceScale{ 1.0f };

    bool IsFirstSync = true;

    DuplicationComponent() = default;
    DuplicationComponent(const Core::UUID& sourceID) : SourceID(sourceID) {}
};
