#pragma once
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct TagComponent
{
    std::string Tag = "Entity";

    TagComponent() = default;
    TagComponent(const std::string& tag)
        : Tag(tag) {
    }
};

struct TransformComponent
{
    glm::vec3 Position{ 0.0f };
    glm::vec3 Rotation{ 0.0f };  // Euler angles in degrees
    glm::vec3 Scale{ 1.0f };

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

    TransformComponent() = default;
    TransformComponent(const glm::vec3& pos)
        : Position(pos) {
    }
};
