#include "core/AssetLoader.hpp"
#include <rendering/sfml/SFMLTexture.hpp>
#include <rendering/sfml/SFMLSprite.hpp>
#include <engine/Audio.hpp>
#include <core/Logger.hpp>
#include <filesystem>
#include <fstream>

namespace RType::Core {

AssetLoader::AssetLoader()
    : initialized(false)
    , debugMode(false)
{
    LOG_DEBUG("AssetLoader", "Created");
}

AssetLoader::~AssetLoader() {
    UnloadAll();
    CleanupSprites();
    LOG_DEBUG("AssetLoader", "Destroyed");
}

bool AssetLoader::Initialize(const std::string& basePathParam) {
    if (!basePathParam.empty()) {
        basePath = basePathParam;
    } else {
        if (!FindBasePath()) {
            LOG_ERROR("AssetLoader", "Could not find base path");
            return false;
        }
    }
    
    // Configurer les chemins de recherche
    searchPaths = {
        "",                    // Current directory
        basePath,              // Base path
        basePath + "game/",    // Game subdirectory
        basePath + "assets/",  // Assets subdirectory
    };
    
    initialized = true;
    LOG_INFO("AssetLoader", "Initialized with base path: " + (basePath.empty() ? "(current dir)" : basePath));
    
    return true;
}

bool AssetLoader::LoadAssetConfig(const std::string& configPath) {
    LOG_INFO("AssetLoader", "Loading asset config from: " + configPath);
    // TODO: Implémenter le chargement depuis Lua
    return true;
}

std::string AssetLoader::ResolveAssetPath(const std::string& relativePath) {
    if (!initialized) {
        LOG_ERROR("AssetLoader", "Not initialized");
        return relativePath;
    }
    
    // Si déjà un chemin absolu, retourner tel quel
    if (std::filesystem::path(relativePath).is_absolute()) {
        return relativePath;
    }
    
    // Chercher le fichier dans les chemins de recherche
    std::string found = FindFile(relativePath);
    if (!found.empty()) {
        return found;
    }
    
    // Fallback: retourner le chemin tel quel
    if (debugMode) {
        LOG_WARNING("AssetLoader", "Could not resolve path: " + relativePath);
    }
    
    return relativePath;
}

void AssetLoader::SetBasePath(const std::string& basePathParam) {
    basePath = basePathParam;
    
    // Mettre à jour les chemins de recherche
    searchPaths = {
        "",
        basePath,
        basePath + "game/",
        basePath + "assets/",
    };
    
    LOG_INFO("AssetLoader", "Base path updated to: " + basePath);
}

std::string AssetLoader::GetBasePath() const {
    return basePath;
}

eng::engine::rendering::ITexture* AssetLoader::LoadTexture(const std::string& name, const std::string& filePath) {
    // Vérifier si déjà chargé
    if (IsTextureLoaded(name)) {
        if (debugMode) {
            LOG_DEBUG("AssetLoader", "Texture already loaded: " + name);
        }
        return textureMap[name];
    }
    
    // Résoudre le chemin
    std::string resolvedPath = ResolveAssetPath(filePath);
    
    // Créer et charger la texture
    auto texture = std::make_unique<eng::engine::rendering::sfml::SFMLTexture>();
    if (!texture->loadFromFile(resolvedPath)) {
        LOG_ERROR("AssetLoader", "Failed to load texture: " + resolvedPath);
        return nullptr;
    }
    
    // Stocker la texture
    eng::engine::rendering::ITexture* texturePtr = texture.get();
    textures[name] = std::move(texture);
    textureMap[name] = texturePtr;
    
    if (debugMode) {
        LOG_INFO("AssetLoader", "Loaded texture: " + name + " (" + resolvedPath + ")");
    }
    
    return texturePtr;
}

eng::engine::rendering::ITexture* AssetLoader::GetTexture(const std::string& name) {
    auto it = textureMap.find(name);
    if (it != textureMap.end()) {
        return it->second;
    }
    return nullptr;
}

bool AssetLoader::PreloadAllTextures() {
    LOG_INFO("AssetLoader", "Preloading all textures...");
    
    // Liste des textures de base à précharger
    struct TextureInfo {
        std::string name;
        std::string path;
    };
    
    std::vector<TextureInfo> defaultTextures = {
        {"background", "game/assets/background.png"},
        {"player", "game/assets/players/r-typesheet42.png"},
        {"missile", "game/assets/players/r-typesheet1.png"},
        {"enemy_bullets", "game/assets/enemies/enemy_bullets.png"},
        {"explosion", "game/assets/enemies/r-typesheet44.png"},
        {"enemy", "game/assets/enemies/r-typesheet5.png"}  // Texture par défaut pour les ennemis (changé de 43 à 5)
    };
    
    bool allLoaded = true;
    for (const auto& info : defaultTextures) {
        if (!LoadTexture(info.name, info.path)) {
            LOG_ERROR("AssetLoader", "Failed to preload texture: " + info.name);
            allLoaded = false;
        }
    }
    
    LOG_INFO("AssetLoader", "Preloaded " + std::to_string(textures.size()) + " textures");
    return allLoaded;
}

std::unordered_map<std::string, eng::engine::rendering::ITexture*>& AssetLoader::GetTextureMap() {
    return textureMap;
}

eng::engine::SoundBuffer* AssetLoader::LoadSoundBuffer(const std::string& name, const std::string& filePath) {
    // Vérifier si déjà chargé
    if (IsSoundLoaded(name)) {
        if (debugMode) {
            LOG_DEBUG("AssetLoader", "Sound already loaded: " + name);
        }
        return soundBuffers[name].get();
    }
    
    // Résoudre le chemin
    std::string resolvedPath = ResolveAssetPath(filePath);
    
    // Créer et charger le buffer
    auto buffer = std::make_unique<eng::engine::SoundBuffer>();
    if (!buffer->loadFromFile(resolvedPath)) {
        if (debugMode) {
            LOG_ERROR("AssetLoader", "Failed to load sound: " + resolvedPath);
        }
        return nullptr;
    }
    
    // Stocker le buffer
    eng::engine::SoundBuffer* bufferPtr = buffer.get();
    soundBuffers[name] = std::move(buffer);
    
    if (debugMode) {
        LOG_INFO("AssetLoader", "Loaded sound: " + name + " (" + resolvedPath + ")");
    }
    
    return bufferPtr;
}

eng::engine::SoundBuffer* AssetLoader::GetSoundBuffer(const std::string& name) {
    auto it = soundBuffers.find(name);
    if (it != soundBuffers.end()) {
        return it->second.get();
    }
    return nullptr;
}

bool AssetLoader::PreloadAllSounds() {
    LOG_INFO("AssetLoader", "Preloading all sounds...");
    
    // Liste des sons de base à précharger
    struct SoundInfo {
        std::string name;
        std::string path;
    };
    
    std::vector<SoundInfo> defaultSounds = {
        {"shoot", "game/assets/vfx/shoot.ogg"},
        {"explosion", "game/assets/vfx/Boom.ogg"},  // Changé de explosion.ogg à Boom.ogg
        {"menu", "game/assets/sounds/Title.ogg"}
    };
    
    for (const auto& info : defaultSounds) {
        if (!LoadSoundBuffer(info.name, info.path)) {
            // Ce n'est pas grave si certains sons ne se chargent pas
            if (debugMode) {
                LOG_WARNING("AssetLoader", "Could not preload sound: " + info.name);
            }
        }
    }
    
    LOG_INFO("AssetLoader", "Preloaded " + std::to_string(soundBuffers.size()) + " sounds");
    return true;  // Toujours retourner true pour les sons
}

eng::engine::rendering::ISprite* AssetLoader::CreateSprite(const std::string& textureName) {
    auto* texture = GetTexture(textureName);
    if (!texture) {
        LOG_ERROR("AssetLoader", "Cannot create sprite: texture not found: " + textureName);
        return nullptr;
    }
    
    auto* sprite = new eng::engine::rendering::sfml::SFMLSprite();
    sprite->setTexture(texture);
    allSprites.push_back(sprite);
    
    if (debugMode) {
        LOG_DEBUG("AssetLoader", "Created sprite for texture: " + textureName);
    }
    
    return sprite;
}

std::vector<eng::engine::rendering::ISprite*>& AssetLoader::GetAllSprites() {
    return allSprites;
}

void AssetLoader::CleanupSprites() {
    for (auto* sprite : allSprites) {
        delete sprite;
    }
    allSprites.clear();
    
    if (debugMode) {
        LOG_DEBUG("AssetLoader", "All sprites cleaned up");
    }
}

void AssetLoader::UnloadTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        textureMap.erase(name);
        textures.erase(it);
        
        if (debugMode) {
            LOG_DEBUG("AssetLoader", "Unloaded texture: " + name);
        }
    }
}

void AssetLoader::UnloadSound(const std::string& name) {
    auto it = soundBuffers.find(name);
    if (it != soundBuffers.end()) {
        soundBuffers.erase(it);
        
        if (debugMode) {
            LOG_DEBUG("AssetLoader", "Unloaded sound: " + name);
        }
    }
}

void AssetLoader::UnloadAll() {
    textures.clear();
    textureMap.clear();
    soundBuffers.clear();
    LOG_INFO("AssetLoader", "All resources unloaded");
}

bool AssetLoader::IsTextureLoaded(const std::string& name) const {
    return textureMap.find(name) != textureMap.end();
}

bool AssetLoader::IsSoundLoaded(const std::string& name) const {
    return soundBuffers.find(name) != soundBuffers.end();
}

AssetLoader::AssetStats AssetLoader::GetStats() const {
    AssetStats stats;
    stats.texturesLoaded = textures.size();
    stats.soundsLoaded = soundBuffers.size();
    stats.spritesCreated = allSprites.size();
    
    // Calculer la mémoire approximative utilisée
    for (const auto& pair : textures) {
        stats.totalMemoryUsed += CalculateTextureMemoryUsage(pair.second.get());
    }
    
    return stats;
}

void AssetLoader::SetDebugMode(bool enabled) {
    debugMode = enabled;
    LOG_INFO("AssetLoader", std::string("Debug mode: ") + (enabled ? "enabled" : "disabled"));
}

void AssetLoader::DebugPrintResources() const {
    LOG_INFO("AssetLoader", "=== Loaded resources ===");
    LOG_INFO("AssetLoader", "Textures (" + std::to_string(textures.size()) + "):");
    for (const auto& pair : textures) {
        LOG_INFO("AssetLoader", "  - " + pair.first);
    }
    
    LOG_INFO("AssetLoader", "Sounds (" + std::to_string(soundBuffers.size()) + "):");
    for (const auto& pair : soundBuffers) {
        LOG_INFO("AssetLoader", "  - " + pair.first);
    }
    
    LOG_INFO("AssetLoader", "Sprites created: " + std::to_string(allSprites.size()));
}

// ========================================
// MÉTHODES PRIVÉES
// ========================================

bool AssetLoader::FindBasePath() {
    // Liste des chemins possibles à tester
    std::vector<std::string> candidates = {
        "",            // Current directory
        "../../",      // Running from build/game/
        "../../../",   // Running from deeper build directories
    };
    
    // Fichier de test pour vérifier qu'on est dans le bon répertoire
    std::string testFile = "game/assets/fonts/Roboto-Regular.ttf";
    
    for (const auto& candidate : candidates) {
        std::string fullPath = candidate + testFile;
        if (std::filesystem::exists(fullPath)) {
            basePath = candidate;
            LOG_INFO("AssetLoader", "Base path found: " + (candidate.empty() ? "(current dir)" : candidate));
            return true;
        }
    }
    
    LOG_ERROR("AssetLoader", "Could not find base path");
    return false;
}

std::string AssetLoader::FindFile(const std::string& relativePath) const {
    for (const auto& searchPath : searchPaths) {
        std::string fullPath = searchPath + relativePath;
        if (std::filesystem::exists(fullPath)) {
            return fullPath;
        }
    }
    
    return "";  // Non trouvé
}

bool AssetLoader::LoadTextureFromConfig(const std::string& name, const std::string& configPath) {
    // TODO: Implémenter le chargement depuis la configuration Lua
    return false;
}

bool AssetLoader::LoadSoundFromConfig(const std::string& name, const std::string& configPath) {
    // TODO: Implémenter le chargement depuis la configuration Lua
    return false;
}

size_t AssetLoader::CalculateTextureMemoryUsage(eng::engine::rendering::ITexture* texture) const {
    if (!texture) return 0;
    
    // Estimation approximative : width * height * 4 bytes (RGBA)
    auto size = texture->getSize();
    return size.x * size.y * 4;
}

} // namespace RType::Core
