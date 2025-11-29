#include "Scene.hpp"
#include "Entity.hpp"

Scene::Scene()
{
}

Entity Scene::CreateEntity(const std::string& name)
{
    entt::entity handle = m_Registry.create();
    return Entity(handle, this);
}

void Scene::DestroyEntity(Entity entity)
{
    m_Registry.destroy(entity.Handle());
}
