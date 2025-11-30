#pragma once
#include <string>
#include <glm/glm.hpp>

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
    glm::vec3 Rotation{ 0.0f };
    glm::vec3 Scale{ 1.0f };

    TransformComponent() = default;
    TransformComponent(const glm::vec3& pos)
        : Position(pos) {
    }
};
