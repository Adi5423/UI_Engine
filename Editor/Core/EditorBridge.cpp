#include "EditorBridge.hpp"

CommandHistory* EditorBridge::s_History = nullptr;

// We need a way to get Scene from Entity. 
// Since we are adding this file, let's fix the Entity class first to allow this 
// OR just implement a helper if we have access.
// Implementation stub - will be fixed by Entity header update.
Scene* EditorBridge::GetScene(Entity entity)
{
    // This requires Entity to have a public getter or friend status.
    // For now, we will add 'Scene* GetScene() const { return m_Scene; }' to Entity.hpp
    return entity.GetScene();
}
