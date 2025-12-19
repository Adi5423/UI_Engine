#pragma once
#include <entt/entt.hpp>

class Scene;

class Entity
{
public:
    Entity() = default;
    Entity(entt::entity handle, Scene* scene)
        : m_Handle(handle), m_Scene(scene) {
    }

    entt::entity Handle() const { return m_Handle; }
    Scene* GetScene() const { return m_Scene; }
    operator bool() const
    {
        return m_Scene && m_Handle != entt::null && m_Scene->Reg().valid(m_Handle);
    }

    template<typename T, typename... Args>
    T& AddComponent(Args&&... args)
    {
        return m_Scene->Reg().emplace<T>(m_Handle, std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    T& AddOrReplaceComponent(Args&&... args)
    {
        return m_Scene->Reg().emplace_or_replace<T>(m_Handle, std::forward<Args>(args)...);
    }

    template<typename T>
    bool HasComponent() const
    {
        return m_Scene->Reg().any_of<T>(m_Handle);
    }

    template<typename T>
    T& GetComponent()
    {
        return m_Scene->Reg().get<T>(m_Handle);
    }

    bool operator==(const Entity& other) const {
        return m_Handle == other.m_Handle && m_Scene == other.m_Scene;
    }

    bool operator!=(const Entity& other) const {
        return !(*this == other);
    }

private:
    entt::entity m_Handle{ entt::null };
    Scene* m_Scene = nullptr;
};
