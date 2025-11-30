#pragma once
#include <entt/entt.hpp>

class Entity;

class Scene
{
public:
    Scene();
    ~Scene() = default;

    Entity CreateEntity(const std::string& name = "Entity");
    void DestroyEntity(Entity entity);

    entt::registry& Reg() { return m_Registry; }

private:
    entt::registry m_Registry;
};
