#include "core/AssetLoader.hpp"
#include <rendering/sfml/SFMLTexture.hpp>
#include <rendering/sfml/SFMLSprite.hpp>
#include <engine/Audio.hpp>
#include <iostream>
#include <filesystem>
#include <fstream>

namespace RType::Core {

AssetLoader::AssetLoader()
    : initialized(false)
    , debugMode(false)
{
}

AssetLoader::~AssetLoader() {
    UnloadAll();
    CleanupSprites();
}

bool AssetLoader::Initialize(const std::string& basePathParam) {
    if (!basePathParam.empty()) {
        basePath = basePathParam;
    } else {
        if (!FindBasePath()) {
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
    
    return true;
}

bool AssetLoader::LoadAssetConfig(const std::string& configPath) {
    // TODO: Implémenter le chargement depuis Lua
    return true;
}

std::string AssetLoader::ResolveAssetPath(const std::string& relativePath) {
    if (!initialized) {
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
}

std::string AssetLoader::GetBasePath() const {
    return basePath;
}

eng::engine::rendering::ITexture* AssetLoader::LoadTexture(const std::string& name, const std::string& filePath) {
    // Vérifier si déjà chargé
    if (IsTextureLoaded(name)) {
        return textureMap[name];
    }
    
    // Résoudre le chemin
    std::string resolvedPath = ResolveAssetPath(filePath);
    
    // Créer et charger la texture
    auto texture = std::make_unique<eng::engine::rendering::sfml::SFMLTexture>();
    if (!texture->loadFromFile(resolvedPath)) {
        return nullptr;
    }
    
    // Stocker la texture
    eng::engine::rendering::ITexture* texturePtr = texture.get();
    textures[name] = std::move(texture);
    textureMap[name] = texturePtr;
    
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
        {"enemy", "game/assets/enemies/r-typesheet43.png"}  // Texture par défaut pour les ennemis
    };
    
    bool allLoaded = true;
    for (const auto& info : defaultTextures) {
        if (!LoadTexture(info.name, info.path)) {
            allLoaded = false;
        }
    }
    
    return allLoaded;
}

std::unordered_map<std::string, eng::engine::rendering::ITexture*>& AssetLoader::GetTextureMap() {
    return textureMap;
}

eng::engine::SoundBuffer* AssetLoader::LoadSoundBuffer(const std::string& name, const std::string& filePath) {
    // Vérifier si déjà chargé
    if (IsSoundLoaded(name)) {
        return soundBuffers[name].get();
    }
    
    // Résoudre le chemin
    std::string resolvedPath = ResolveAssetPath(filePath);
    
    // Créer et charger le buffer
    auto buffer = std::make_unique<eng::engine::SoundBuffer>();
    if (!buffer->loadFromFile(resolvedPath)) {
        return nullptr;
    }
    
    // Stocker le buffer
    eng::engine::SoundBuffer* bufferPtr = buffer.get();
    soundBuffers[name] = std::move(buffer);
    
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
    // Liste des sons de base à précharger
    struct SoundInfo {
        std::string name;
        std::string path;
    };
    
    std::vector<SoundInfo> defaultSounds = {
        {"shoot", "game/assets/vfx/shoot.ogg"},
        {"explosion", "game/assets/vfx/explosion.ogg"},
        {"menu", "game/assets/sounds/Title.ogg"}
    };
    
    bool allLoaded = true;
    for (const auto& info : defaultSounds) {
        if (!LoadSoundBuffer(info.name, info.path)) {
            // Ce n'est pas grave si certains sons ne se chargent pas
        }
    }
    
    return true;  // Toujours retourner true pour les sons
}

eng::engine::rendering::ISprite* AssetLoader::CreateSprite(const std::string& textureName) {
    auto* texture = GetTexture(textureName);
    if (!texture) {
        return nullptr;
    }
    
    auto* sprite = new eng::engine::rendering::sfml::SFMLSprite();
    sprite->setTexture(texture);
    allSprites.push_back(sprite);
    
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
}

void AssetLoader::UnloadTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        textureMap.erase(name);
        textures.erase(it);
    }
}

void AssetLoader::UnloadSound(const std::string& name) {
    auto it = soundBuffers.find(name);
    if (it != soundBuffers.end()) {
        soundBuffers.erase(it);
    }
}

void AssetLoader::UnloadAll() {
    textures.clear();
    textureMap.clear();
    soundBuffers.clear();
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
    std::cout << "[AssetLoader] Debug mode: " << (enabled ? "enabled" : "disabled") << std::endl;
}

void AssetLoader::DebugPrintResources() const {
    std::cout << "[AssetLoader] Loaded resources:" << std::endl;
    
    std::cout << "  Textures (" << textures.size() << "):" << std::endl;
    for (const auto& pair : textures) {
        std::cout << "    - " << pair.first << std::endl;
    }
    
    std::cout << "  Sounds (" << soundBuffers.size() << "):" << std::endl;
    for (const auto& pair : soundBuffers) {
        std::cout << "    - " << pair.first << std::endl;
    }
    
    std::cout << "  Sprites created: " << allSprites.size() << std::endl;
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
            std::cout << "[AssetLoader] Base path found: " 
                      << (candidate.empty() ? "(current dir)" : candidate) << std::endl;
            return true;
        }
    }
    
    std::cerr << "[AssetLoader] Could not find base path" << std::endl;
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
