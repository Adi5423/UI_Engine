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

        if (reg.any_of<CameraComponent>(handle))
        {
            m_HasCamera = true;
            m_CameraComp = reg.get<CameraComponent>(handle);
        }

        if (reg.any_of<DuplicationComponent>(handle))
        {
            m_HasDuplication = true;
            m_DuplicationComp = reg.get<DuplicationComponent>(handle);
        }

        if (reg.any_of<HierarchyOrderComponent>(handle))
            m_HierarchyComp = reg.get<HierarchyOrderComponent>(handle);
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

        if (m_HasCamera)
        {
            restored.AddComponent<CameraComponent>() = m_CameraComp;
        }

        if (m_HasDuplication)
        {
            restored.AddComponent<DuplicationComponent>() = m_DuplicationComp;
        }

        if (!restored.HasComponent<HierarchyOrderComponent>())
            restored.AddComponent<HierarchyOrderComponent>();
        restored.GetComponent<HierarchyOrderComponent>() = m_HierarchyComp;
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
    HierarchyOrderComponent m_HierarchyComp;

    bool m_HasCamera = false;
    CameraComponent m_CameraComp;

    bool m_HasDuplication = false;
    DuplicationComponent m_DuplicationComp;
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
// =========================================================================================
// DUPLICATE ENTITY COMMAND
// =========================================================================================
class DuplicateEntityCommand : public ICommand
{
public:
    DuplicateEntityCommand(Scene* scene, Entity source, bool isLinked = false)
        : m_Scene(scene), m_SourceUUID(source.GetComponent<IDComponent>().ID), m_IsLinked(isLinked)
    {
        m_NewEntityUUID = Core::UUID(); // Assign new ID for the duplicate
        if (source.HasComponent<TagComponent>())
            m_SourceName = source.GetComponent<TagComponent>().Tag;
    }

    void Execute() override
    {
        if (!m_Scene) return;
        Entity source = m_Scene->GetEntityByUUID(m_SourceUUID);
        if (!source) return;

        // Perform duplication with a stable UUID so Undo/Redo is deterministic.
        SceneAPI::DuplicateEntityWithUUID(*m_Scene, source, m_NewEntityUUID, m_IsLinked);
    }

    void Undo() override
    {
        if (!m_Scene) return;
        Entity entity = m_Scene->GetEntityByUUID(m_NewEntityUUID);
        if (entity)
        {
            m_Scene->DestroyEntity(entity);
        }
    }

    std::string GetDescription() const override
    {
        return "Duplicate " + m_SourceName;
    }

private:
    Scene* m_Scene;
    Core::UUID m_SourceUUID;
    Core::UUID m_NewEntityUUID;
    std::string m_SourceName;
    bool m_IsLinked;
};
// =========================================================================================
// REORDER ENTITY COMMAND
// =========================================================================================
class ReorderEntityCommand : public ICommand
{
public:
    ReorderEntityCommand(Scene* scene, Entity entity, int32_t oldOrder, int32_t newOrder)
        : m_Scene(scene), m_OldOrder(oldOrder), m_NewOrder(newOrder)
    {
        if (entity && entity.HasComponent<IDComponent>())
            m_EntityUUID = entity.GetComponent<IDComponent>().ID;
    }

    void Execute() override
    {
        Entity entity = m_Scene->GetEntityByUUID(m_EntityUUID);
        if (entity)
        {
            entity.AddOrReplaceComponent<HierarchyOrderComponent>(m_NewOrder);
        }
    }

    void Undo() override
    {
        Entity entity = m_Scene->GetEntityByUUID(m_EntityUUID);
        if (entity)
        {
            entity.AddOrReplaceComponent<HierarchyOrderComponent>(m_OldOrder);
        }
    }

    std::string GetDescription() const override
    {
        return "Reorder Entity";
    }

private:
    Scene* m_Scene;
    Core::UUID m_EntityUUID;
    int32_t m_OldOrder;
    int32_t m_NewOrder;
};
