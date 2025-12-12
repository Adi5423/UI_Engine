#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <Rendering/Shaders/Shader.hpp>
#include <Rendering/Texture.hpp>

/**
 * ============================================================================
 * RESOURCE MANAGER
 * ============================================================================
 * 
 * Professional singleton-based resource management.
 * Handles loading, caching, and retrieval of assets.
 * 
 * Features:
 * - Automatic caching (prevents duplicate loads)
 * - Reference counting (via shared_ptr)
 * - Centralized asset access
 * 
 * Usage:
 * auto shader = ResourceManager::LoadShader("FlatColor", "assets/shaders/flat.glsl");
 * auto texture = ResourceManager::LoadTexture("Wood", "assets/textures/wood.png");
 * 
 * ============================================================================
 */

class ResourceManager {
public:
    // Shaders
    static std::shared_ptr<Shader> LoadShader(const std::string& name, const std::string& vertexSource, const std::string& fragmentSource);
    static std::shared_ptr<Shader> GetShader(const std::string& name);

    // Textures (Backbone ready)
    static std::shared_ptr<Texture2D> LoadTexture(const std::string& name, const std::string& path);
    static std::shared_ptr<Texture2D> GetTexture(const std::string& name);

    // Models (Backbone ready - to be implemented)
    // static std::shared_ptr<Model> LoadModel(const std::string& name, const std::string& path);

    static void Clear();

private:
    // Private constructor (Static class)
    ResourceManager() {}

    // Methods to actually load from file
    static std::shared_ptr<Shader> LoadShaderFromFile(const std::string& vShaderFile, const std::string& fShaderFile);
    static std::shared_ptr<Texture2D> LoadTextureFromFile(const std::string& file);

    // Storage Cache
    static std::unordered_map<std::string, std::shared_ptr<Shader>> s_Shaders;
    static std::unordered_map<std::string, std::shared_ptr<Texture2D>> s_Textures;
};
