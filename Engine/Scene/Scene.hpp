#pragma once
#include <entt/entt.hpp>

#include <Core/UUID.hpp>

class Entity;

class Scene
{
public:
    Scene();
    ~Scene() = default;

    Entity CreateEntity(const std::string& name = "Entity");
    Entity CreateEntityWithUUID(Core::UUID uuid, const std::string& name = "Entity");
    void DestroyEntity(Entity entity);

    void OnUpdate(float ts);
    
    Entity GetEntityByUUID(Core::UUID uuid);

    entt::registry& Reg() { return m_Registry; }

private:
    entt::registry m_Registry;
};
