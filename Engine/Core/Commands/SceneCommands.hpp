#pragma once

#include "Command.hpp"
#include <Scene/Scene.hpp>
#include <Scene/Entity.hpp>
#include <Scene/Components.hpp>
#include <Scene/SceneAPI.hpp>
#include <Rendering/Mesh/Mesh.hpp>
#include <memory>
#include <Core/UUID.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

// =========================================================================================
// CREATE MESH COMMAND
// =========================================================================================
class CreateMeshCommand : public ICommand
{
public:
    CreateMeshCommand(Scene* scene, const std::string& name, std::shared_ptr<Mesh> mesh, const glm::vec3& position = {0.0f, 0.0f, 0.0f})
        : m_Scene(scene), m_Name(name), m_Mesh(mesh), m_Position(position)
    {
        // Assign a stable UUID for this entity, persisting across Undo/Redo
        m_EntityUUID = Core::UUID(); 
    }

    void Execute() override
    {
        if (!m_Scene) return;
        
        // Use the STABLE UUID to create/recreate the entity
        // Check if it already exists (sanity check, though Execute implies it shouldn't)
        Entity existing = m_Scene->GetEntityByUUID(m_EntityUUID);
        if (existing)
        {
            return;
        }

        SceneAPI::CreateMeshEntityWithUUID(*m_Scene, m_EntityUUID, m_Name, m_Mesh, m_Position);
    }

    void Undo() override
    {
        if (!m_Scene) return;
        
        Entity entity = m_Scene->GetEntityByUUID(m_EntityUUID);
        if (entity)
        {
            m_Scene->DestroyEntity(entity);
        }
    }

    std::string GetDescription() const override
    {
        return "Create " + m_Name;
    }

private:
    Scene* m_Scene;
    std::string m_Name;
    std::shared_ptr<Mesh> m_Mesh;
    glm::vec3 m_Position;
    Core::UUID m_EntityUUID;
};


// =========================================================================================
// DELETE ENTITY COMMAND
// =========================================================================================
class DeleteEntityCommand : public ICommand
{
public:
    DeleteEntityCommand(Scene* scene, Entity entity)
        : m_Scene(scene)
    {
        if (entity.HasComponent<IDComponent>())
            m_EntityUUID = entity.GetComponent<IDComponent>().ID;

        // Capture State
        auto& reg = m_Scene->Reg();
        entt::entity handle = entity.Handle();
        
        if (reg.any_of<TagComponent>(handle))
            m_TagComp = reg.get<TagComponent>(handle);
            
        if (reg.any_of<TransformComponent>(handle))
            m_TransformComp = reg.get<TransformComponent>(handle);
            
        if (reg.any_of<MeshComponent>(handle))
            m_MeshComp = reg.get<MeshComponent>(handle);
    }

    void Execute() override
    {
        Entity entity = m_Scene->GetEntityByUUID(m_EntityUUID);
        if (entity)
        {
            m_Scene->DestroyEntity(entity);
        }
    }

    void Undo() override
    {
        if (!m_Scene) return;
        
        // Restore with SAME UUID
        Entity restored = m_Scene->CreateEntityWithUUID(m_EntityUUID, m_TagComp.Tag);
        
        // Restore TagComponent (CreateEntityWithUUID does not add it by default to avoid conflicts)
        if (!restored.HasComponent<TagComponent>())
            restored.AddComponent<TagComponent>(m_TagComp.Tag);
        else
            restored.GetComponent<TagComponent>() = m_TagComp;

        // Restore components logic
        if (!restored.HasComponent<TransformComponent>())
            restored.AddComponent<TransformComponent>();
        restored.GetComponent<TransformComponent>() = m_TransformComp;
        
        if (m_MeshComp.MeshHandle)
        {
            if (!restored.HasComponent<MeshComponent>())
                restored.AddComponent<MeshComponent>();
            restored.GetComponent<MeshComponent>() = m_MeshComp;
        }
    }

    std::string GetDescription() const override
    {
        return "Delete " + m_TagComp.Tag;
    }

private:
    Scene* m_Scene;
    Core::UUID m_EntityUUID;
    
    TagComponent       m_TagComp;
    TransformComponent m_TransformComp;
    MeshComponent      m_MeshComp;
};


// =========================================================================================
// MODIFY TRANSFORM COMMAND
// =========================================================================================
class ModifyTransformCommand : public ICommand
{
public:
    ModifyTransformCommand(Scene* scene, Entity entity, const TransformComponent& oldFn, const TransformComponent& newFn)
        : m_Scene(scene), 
          m_OldTransform(oldFn), 
          m_NewTransform(newFn)
    {
         if (entity && entity.HasComponent<TagComponent>())
            m_EntityName = entity.GetComponent<TagComponent>().Tag;
            
         if (entity && entity.HasComponent<IDComponent>())
            m_EntityUUID = entity.GetComponent<IDComponent>().ID;
    }

    void Execute() override
    {
        Entity entity = m_Scene->GetEntityByUUID(m_EntityUUID);
        if (!entity)
        {
            return;
        }
        
        auto& tc = entity.GetComponent<TransformComponent>();
        tc = m_NewTransform;
    }

    void Undo() override
    {
        Entity entity = m_Scene->GetEntityByUUID(m_EntityUUID);
        if (!entity)
        {
            return;
        }
        
        auto& tc = entity.GetComponent<TransformComponent>();
        tc = m_OldTransform;
    }

    std::string GetDescription() const override
    {
        return "Transform " + m_EntityName;
    }

private:
    Scene* m_Scene;
    Core::UUID m_EntityUUID;
    std::string m_EntityName;
    
    TransformComponent m_OldTransform;
    TransformComponent m_NewTransform;
};


// =========================================================================================
// RENAME ENTITY COMMAND
// =========================================================================================
class RenameEntityCommand : public ICommand
{
public:
    RenameEntityCommand(Scene* scene, Entity entity, const std::string& oldName, const std::string& newName)
        : m_Scene(scene), m_OldName(oldName), m_NewName(newName)
    {
        if (entity.HasComponent<IDComponent>())
            m_EntityUUID = entity.GetComponent<IDComponent>().ID;
    }

    void Execute() override
    {
        Entity entity = m_Scene->GetEntityByUUID(m_EntityUUID);
        if (entity)
            entity.GetComponent<TagComponent>().Tag = m_NewName;
    }

    void Undo() override
    {
        Entity entity = m_Scene->GetEntityByUUID(m_EntityUUID);
        if (entity)
            entity.GetComponent<TagComponent>().Tag = m_OldName;
    }

    std::string GetDescription() const override
    {
        return "Rename " + m_OldName + " -> " + m_NewName;
    }

private:
    Scene* m_Scene;
    Core::UUID m_EntityUUID;
    std::string m_OldName;
    std::string m_NewName;
};
