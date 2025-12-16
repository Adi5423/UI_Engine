#include "Scene.hpp"
#include "Entity.hpp"
#include "Components.hpp"

Scene::Scene()
{
}

Entity Scene::CreateEntity(const std::string& name)
{
    return CreateEntityWithUUID(Core::UUID(), name);
}

Entity Scene::CreateEntityWithUUID(Core::UUID uuid, const std::string& name)
{
    Entity entity = { m_Registry.create(), this };
    
    // Add ID Component
    entity.AddComponent<IDComponent>(uuid);
    
    // Check if TagComponent is needed. 
    // SceneAPI adds it manually. 
    // To be safe and compliant with existing SceneAPI, we WON'T add TagComponent here 
    // UNLESS the previous implementation did. 
    // Previous implementation ignored 'name'.
    // BUT common practice is CreateEntity ADDS Tag.
    // If I add Tag here, SceneAPI will crash on emplace.
    // So I will update SceneAPI to NOT add Tag if it exists, OR I won't add Tag here.
    
    // Given the user wants "Change whole project structure", I should make CreateEntity standard.
    // Standard: CreateEntity adds Tag and Transform usually.
    // But SceneAPI is explicit.
    
    // Let's stick to MINIMAL INTERFERENCE with SceneAPI logic for now regarding Tags.
    // BUT we MUST add IDComponent.
    
    return entity;
}

void Scene::DestroyEntity(Entity entity)
{
    m_Registry.destroy(entity.Handle());
}

void Scene::OnUpdate(float ts)
{
    // Update scripts, physics, etc. here in the future.
}

Entity Scene::GetEntityByUUID(Core::UUID uuid)
{
    auto view = m_Registry.view<IDComponent>();
    for (auto entity : view)
    {
        const auto& id = view.get<IDComponent>(entity).ID;
        if (id == uuid)
            return Entity{ entity, this };
    }
    return Entity{};
}
