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
    
    // Register in map for O(1) lookup
    m_EntityMap[uuid] = entity.Handle();
    
    return entity;
}

void Scene::DestroyEntity(Entity entity)
{
    if (!entity) return;

    // Remove from map before destroying
    if (entity.HasComponent<IDComponent>())
    {
        m_EntityMap.erase(entity.GetComponent<IDComponent>().ID);
    }

    m_Registry.destroy(entity.Handle());
}

void Scene::OnUpdate(float ts)
{
    // -------------------------------------------------------------------------
    // Duplication Sync Logic (Delta Propagation)
    // -------------------------------------------------------------------------
    auto view = m_Registry.view<TransformComponent, DuplicationComponent>();
    for (auto entity : view)
    {
        auto& tc = view.get<TransformComponent>(entity);
        auto& dup = view.get<DuplicationComponent>(entity);

        Entity source = GetEntityByUUID(dup.SourceID);
        if (source && source.HasComponent<TransformComponent>())
        {
            auto& sourceTC = source.GetComponent<TransformComponent>();

            // If it's the first sync, just cache the current state to avoid immediate jumps
            if (dup.IsFirstSync)
            {
                dup.LastSourcePosition = sourceTC.Position;
                dup.LastSourceRotation = sourceTC.Rotation;
                dup.LastSourceScale = sourceTC.Scale;
                dup.IsFirstSync = false;
            }
            else
            {
                // Calculate Delta from last known source state
                glm::vec3 posDelta = sourceTC.Position - dup.LastSourcePosition;
                glm::vec3 rotDelta = sourceTC.Rotation - dup.LastSourceRotation;
                glm::vec3 scaleDelta = sourceTC.Scale - dup.LastSourceScale;

                // If source moved, apply delta to duplicate
                bool changed = false;
                if (glm::length(posDelta) > 0.0001f) { tc.Position += posDelta; changed = true; }
                if (glm::length(rotDelta) > 0.0001f) { tc.Rotation += rotDelta; changed = true; }
                if (glm::length(scaleDelta) > 0.0001f) { tc.Scale += scaleDelta; changed = true; }

                if (changed)
                {
                    // Update cache for next frame's delta calculation
                    dup.LastSourcePosition = sourceTC.Position;
                    dup.LastSourceRotation = sourceTC.Rotation;
                    dup.LastSourceScale = sourceTC.Scale;
                }
            }
        }
    }
}

Entity Scene::GetEntityByUUID(Core::UUID uuid)
{
    if (m_EntityMap.find(uuid) != m_EntityMap.end())
    {
        entt::entity handle = m_EntityMap.at(uuid);
        if (m_Registry.valid(handle))
            return Entity{ handle, this };
        else
            m_EntityMap.erase(uuid); // Clean up stale entry
    }
    
    // Fallback: search just in case (and repopulate map)
    auto view = m_Registry.view<IDComponent>();
    for (auto entity : view)
    {
        const auto& id = view.get<IDComponent>(entity).ID;
        if (id == uuid)
        {
            m_EntityMap[uuid] = entity;
            return Entity{ entity, this };
        }
    }

    return Entity{};
}
