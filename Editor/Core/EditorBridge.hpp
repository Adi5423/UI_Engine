#pragma once

#include <memory>
#include <string>
#include <glm/glm.hpp>
#include <Scene/Scene.hpp>
#include <Scene/Entity.hpp>
#include <Scene/Components.hpp>
#include <Core/Commands/CommandHistory.hpp>
#include <Core/Commands/SceneCommands.hpp>
#include <Core/Logger.hpp>

/**
 * EditorBridge aka "The Wall"
 * 
 * Acts as the mandatory gateway between the UI (Frontend) and the Engine/Data (Backend).
 * Any modification from the Inspector or Viewport MUST pass through here.
 * This ensures:
 * 1. Consistent Logging
 * 2. Guaranteed Command Creation for Undo/Redo
 * 3. Centralized debugging point
 */
class EditorBridge
{
public:
    static void Init(CommandHistory* history)
    {
        s_History = history;
    }

    // =================================================================================
    // TRANSFORM OPERATIONS
    // =================================================================================
    static void SubmitTransformChange(Entity entity, const TransformComponent& oldTransform, const TransformComponent& newTransform)
    {
        if (!s_History) { Logger::Error("[Bridge] History not initialized!"); return; }
        if (!entity) return;

        // Validation: Don't spam commands if nothing changed (floating point epsilon check could be added here)
        if (oldTransform.Position == newTransform.Position &&
            oldTransform.Rotation == newTransform.Rotation &&
            oldTransform.Scale == newTransform.Scale)
        {
            return;
        }

        Logger::Info("[Bridge] Transform Change Submitted for Entity: " + std::to_string((uint32_t)entity.Handle()));

        auto cmd = std::make_unique<ModifyTransformCommand>(
            GetScene(entity),
            entity,
            oldTransform,
            newTransform
        );
        s_History->ExecuteCommand(std::move(cmd));
    }

    // =================================================================================
    // DELETE OPERATIONS
    // =================================================================================
    static void SubmitDeleteEntity(Entity entity)
    {
        if (!s_History) return;
        if (!entity) return;

        Logger::Info("[Bridge] Delete Request for Entity: " + std::to_string((uint32_t)entity.Handle()));

        auto cmd = std::make_unique<DeleteEntityCommand>(
            GetScene(entity),
            entity
        );
        s_History->ExecuteCommand(std::move(cmd));
    }

    // =================================================================================
    // CREATE OPERATIONS
    // =================================================================================
    static void SubmitCreateMesh(Scene* scene, const std::string& name, const std::shared_ptr<Mesh>& mesh)
    {
        if (!s_History || !scene) return;

        Logger::Info("[Bridge] Create Mesh Request: " + name);

        auto cmd = std::make_unique<CreateMeshCommand>(
            scene,
            name,
            mesh,
            glm::vec3(0.0f)
        );
        s_History->ExecuteCommand(std::move(cmd));
    }

    // =================================================================================
    // RENAME OPERATIONS
    // =================================================================================
    static void SubmitRename(Entity entity, const std::string& oldName, const std::string& newName)
    {
        if (!s_History) return;
        if (oldName == newName) return;

        Logger::Info("[Bridge] Rename Request: " + oldName + " -> " + newName);

        auto cmd = std::make_unique<RenameEntityCommand>(
            GetScene(entity),
            entity,
            oldName,
            newName
        );
        s_History->ExecuteCommand(std::move(cmd));
    }

private:
    static CommandHistory* s_History;

    // Helper to extract Scene* from Entity wrapper safely
    // Since Entity class has m_Scene (private), we typically have it or pass it.
    // However, the Command constructors take Scene*.
    // Using a hack or just relying on context. 
    // Ideally Entity should expose GetScene(). 
    // For now we will rely on the caller passing scene or assuming the Entity interface has access?
    // Use the stored scene pointer if valid.
    // Actually, Entity DOES NOT expose GetScene() publicly in the provided file.
    // But since the user code sends m_ActiveScene usually, we might need to change signature slightly if we can't extract it.
    // Let's assume we can't extract it easily without Friend class.
    // BUT! ModifyTransformCommand takes Scene*.
    // The previous code used `m_ActiveScene.get()` in EditorLayer.
    // We will cheat and assume we can pass `entity.m_Scene` via friend or strict binding, OR simpler:
    // Update Entity to expose it, or pass scene in arguments.
    // Let's UPDATE Entity.hpp to expose GetScene().
    static Scene* GetScene(Entity entity);
};
