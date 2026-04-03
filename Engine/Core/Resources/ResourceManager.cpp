#include "ResourceManager.hpp"
#include <iostream>
#include <Core/Log.hpp>

// Define static storage
std::unordered_map<std::string, std::shared_ptr<Shader>> ResourceManager::s_Shaders;
std::unordered_map<std::string, std::shared_ptr<Texture2D>> ResourceManager::s_Textures;
std::mutex ResourceManager::s_Mutex;

std::shared_ptr<Shader> ResourceManager::LoadShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource)
{
    std::lock_guard<std::mutex> lock(s_Mutex);

    // Check if exists
    auto it = s_Shaders.find(name);
    if (it != s_Shaders.end()) {
        CORE_WARN("ResourceManager: Shader '", name, "' already exists! Returning cached.");
        return it->second;
    }

    std::shared_ptr<Shader> shader = std::make_shared<Shader>(vertexSource, fragmentSource);
    s_Shaders[name] = shader;
    
    CORE_INFO("ResourceManager: Loaded Shader '", name, "'");
    return shader;
}

std::shared_ptr<Shader> ResourceManager::GetShader(const std::string& name)
{
    std::lock_guard<std::mutex> lock(s_Mutex);

    auto it = s_Shaders.find(name);
    if (it == s_Shaders.end()) {
        CORE_ERROR("ResourceManager: Shader '", name, "' not found!");
        return nullptr;
    }
    return it->second;
}

std::shared_ptr<Texture2D> ResourceManager::LoadTexture(const std::string& name, const std::string& path)
{
    std::lock_guard<std::mutex> lock(s_Mutex);

    auto it = s_Textures.find(name);
    if (it != s_Textures.end()) {
        CORE_WARN("ResourceManager: Texture '", name, "' already exists! Returning cached.");
        return it->second;
    }

    // MED-07 FIX: Log warning that texture loading is not yet implemented
    CORE_WARN("ResourceManager: Texture loading not yet implemented. '", name, "' will be null.");

    // Backbone implementation: Logic would go here
    // std::shared_ptr<Texture2D> texture = Texture2D::Create(path);
    // s_Textures[name] = texture;
    
    return nullptr; 
}

std::shared_ptr<Texture2D> ResourceManager::GetTexture(const std::string& name)
{
    std::lock_guard<std::mutex> lock(s_Mutex);

    auto it = s_Textures.find(name);
    if (it == s_Textures.end()) {
        CORE_ERROR("ResourceManager: Texture '", name, "' not found!");
        return nullptr;
    }
    return it->second;
}

void ResourceManager::Clear()
{
    std::lock_guard<std::mutex> lock(s_Mutex);
    s_Shaders.clear();
    s_Textures.clear();
}
