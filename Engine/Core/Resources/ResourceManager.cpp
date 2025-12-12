#include "ResourceManager.hpp"
#include <iostream>
#include <Core/Log.hpp>

// Define static storage
std::unordered_map<std::string, std::shared_ptr<Shader>> ResourceManager::s_Shaders;
std::unordered_map<std::string, std::shared_ptr<Texture2D>> ResourceManager::s_Textures;

std::shared_ptr<Shader> ResourceManager::LoadShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource)
{
    // Check if exists
    if (s_Shaders.find(name) != s_Shaders.end()) {
        CORE_WARN("ResourceManager: Shader '{0}' already exists! Returning cached.", name);
        return s_Shaders[name];
    }

    // Load (Create shader from strings for now, or file if extended)
    // Note: The signature implies source strings. 
    // If we want file loading, we'd read files here. 
    // For now, mirroring current Shader constructor which takes strings.
    
    std::shared_ptr<Shader> shader = std::make_shared<Shader>(vertexSource, fragmentSource);
    s_Shaders[name] = shader;
    
    CORE_INFO("ResourceManager: Loaded Shader '{0}'", name);
    return shader;
}

std::shared_ptr<Shader> ResourceManager::GetShader(const std::string& name)
{
    if (s_Shaders.find(name) == s_Shaders.end()) {
        CORE_ERROR("ResourceManager: Shader '{0}' not found!", name);
        return nullptr;
    }
    return s_Shaders[name];
}

std::shared_ptr<Texture2D> ResourceManager::LoadTexture(const std::string& name, const std::string& path)
{
    if (s_Textures.find(name) != s_Textures.end()) {
        CORE_WARN("ResourceManager: Texture '{0}' already exists! Returning cached.", name);
        return s_Textures[name];
    }

    // Backbone implementation: Logic would go here
    // std::shared_ptr<Texture2D> texture = Texture2D::Create(path);
    // s_Textures[name] = texture;
    
    CORE_INFO("ResourceManager: Loaded Texture '{0}' (Backbone: Simulated)", name);
    
    // Return empty for now as requested (stub)
    return nullptr; 
}

std::shared_ptr<Texture2D> ResourceManager::GetTexture(const std::string& name)
{
    if (s_Textures.find(name) == s_Textures.end()) {
        CORE_ERROR("ResourceManager: Texture '{0}' not found!", name);
        return nullptr;
    }
    return s_Textures[name];
}

void ResourceManager::Clear()
{
    s_Shaders.clear();
    s_Textures.clear();
}
