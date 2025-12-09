#pragma once
#include <string>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Rendering/Mesh/Mesh.hpp>

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

        glm::mat4 R =
            glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.y), {0,1,0}) *
            glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.x), {1,0,0}) *
            glm::rotate(glm::mat4(1.0f), glm::radians(Rotation.z), {0,0,1});

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
