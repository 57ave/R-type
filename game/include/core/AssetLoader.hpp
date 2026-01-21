#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include <engine/Audio.hpp>
#include <rendering/ITexture.hpp>
#include <rendering/ISprite.hpp>
#include <rendering/sfml/SFMLTexture.hpp>
#include <rendering/sfml/SFMLSprite.hpp>

namespace RType::Core {

/**
 * @brief Gestionnaire centralisé des ressources (textures, sons, etc.)
 * 
 * Cette classe gère :
 * - Le chargement des textures
 * - Le chargement des sons
 * - Le cache des ressources
 * - La résolution des chemins d'assets
 * - Le préchargement des ressources
 */
class AssetLoader {
public:
    AssetLoader();
    ~AssetLoader();

    // ========================================
    // INITIALISATION
    // ========================================
    
    /**
     * @brief Initialise le loader avec le chemin de base
     * @param basePath Chemin de base vers les assets
     */
    bool Initialize(const std::string& basePath);
    
    /**
     * @brief Charge la configuration des assets depuis Lua
     * @param configPath Chemin vers la configuration
     */
    bool LoadAssetConfig(const std::string& configPath);

    // ========================================
    // RÉSOLUTION DE CHEMINS
    // ========================================
    
    /**
     * @brief Résout un chemin d'asset relatif vers un chemin absolu
     * @param relativePath Chemin relatif
     * @return Chemin absolu résolu
     */
    std::string ResolveAssetPath(const std::string& relativePath);
    
    /**
     * @brief Définit le chemin de base des assets
     * @param basePath Nouveau chemin de base
     */
    void SetBasePath(const std::string& basePath);
    
    /**
     * @brief Obtient le chemin de base actuel
     */
    std::string GetBasePath() const;

    // ========================================
    // CHARGEMENT DE TEXTURES
    // ========================================
    
    /**
     * @brief Charge une texture
     * @param name Nom de la texture (pour le cache)
     * @param filePath Chemin vers le fichier
     * @return Pointeur vers la texture ou nullptr si échec
     */
    eng::engine::rendering::ITexture* LoadTexture(const std::string& name, const std::string& filePath);
    
    /**
     * @brief Obtient une texture chargée
     * @param name Nom de la texture
     * @return Pointeur vers la texture ou nullptr si non trouvée
     */
    eng::engine::rendering::ITexture* GetTexture(const std::string& name);
    
    /**
     * @brief Précharge toutes les textures définies dans la configuration
     */
    bool PreloadAllTextures();
    
    /**
     * @brief Obtient la map complète des textures
     */
    std::unordered_map<std::string, eng::engine::rendering::ITexture*>& GetTextureMap();

    // ========================================
    // CHARGEMENT DE SONS
    // ========================================
    
    /**
     * @brief Charge un buffer audio
     * @param name Nom du son (pour le cache)
     * @param filePath Chemin vers le fichier
     * @return Pointeur vers le buffer ou nullptr si échec
     */
    eng::engine::SoundBuffer* LoadSoundBuffer(const std::string& name, const std::string& filePath);
    
    /**
     * @brief Obtient un buffer audio chargé
     * @param name Nom du son
     * @return Pointeur vers le buffer ou nullptr si non trouvé
     */
    eng::engine::SoundBuffer* GetSoundBuffer(const std::string& name);
    
    /**
     * @brief Précharge tous les sons définis dans la configuration
     */
    bool PreloadAllSounds();

    // ========================================
    // GESTION DES SPRITES
    // ========================================
    
    /**
     * @brief Créé un nouveau sprite et l'enregistre pour le nettoyage
     * @param textureName Nom de la texture à utiliser
     * @return Pointeur vers le sprite créé
     */
    eng::engine::rendering::ISprite* CreateSprite(const std::string& textureName);
    
    /**
     * @brief Obtient la liste de tous les sprites créés
     */
    std::vector<eng::engine::rendering::ISprite*>& GetAllSprites();
    
    /**
     * @brief Nettoie tous les sprites créés
     */
    void CleanupSprites();

    // ========================================
    // GESTION MÉMOIRE
    // ========================================
    
    /**
     * @brief Décharge une texture spécifique
     * @param name Nom de la texture à décharger
     */
    void UnloadTexture(const std::string& name);
    
    /**
     * @brief Décharge un son spécifique
     * @param name Nom du son à décharger
     */
    void UnloadSound(const std::string& name);
    
    /**
     * @brief Décharge toutes les ressources
     */
    void UnloadAll();

    // ========================================
    // UTILITAIRES
    // ========================================
    
    /**
     * @brief Vérifie si une texture est chargée
     * @param name Nom de la texture
     */
    bool IsTextureLoaded(const std::string& name) const;
    
    /**
     * @brief Vérifie si un son est chargé
     * @param name Nom du son
     */
    bool IsSoundLoaded(const std::string& name) const;
    
    /**
     * @brief Obtient des statistiques sur les ressources chargées
     */
    struct AssetStats {
        size_t texturesLoaded = 0;
        size_t soundsLoaded = 0;
        size_t spritesCreated = 0;
        size_t totalMemoryUsed = 0;  // En bytes (approximatif)
    };
    
    AssetStats GetStats() const;
    
    /**
     * @brief Active/désactive les logs de debug
     */
    void SetDebugMode(bool enabled);
    
    /**
     * @brief Affiche la liste de toutes les ressources chargées
     */
    void DebugPrintResources() const;

private:
    // Stockage des ressources
    std::unordered_map<std::string, std::unique_ptr<eng::engine::rendering::sfml::SFMLTexture>> textures;
    std::unordered_map<std::string, eng::engine::rendering::ITexture*> textureMap;  // Pour compatibilité
    std::unordered_map<std::string, std::unique_ptr<eng::engine::SoundBuffer>> soundBuffers;
    std::vector<eng::engine::rendering::ISprite*> allSprites;
    
    // Configuration
    std::string basePath;
    bool initialized;
    bool debugMode;
    
    // Chemins de recherche possibles
    std::vector<std::string> searchPaths;
    
    // Méthodes privées
    bool FindBasePath();
    std::string FindFile(const std::string& relativePath) const;
    bool LoadTextureFromConfig(const std::string& name, const std::string& configPath);
    bool LoadSoundFromConfig(const std::string& name, const std::string& configPath);
    size_t CalculateTextureMemoryUsage(eng::engine::rendering::ITexture* texture) const;
};

} // namespace RType::Core
