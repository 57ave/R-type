#pragma once

#include <memory>
#include <unordered_map>
#include <string>
#include <engine/Audio.hpp>

namespace RType::Core {

/**
 * @brief Structure pour les données d'un track musical
 */
struct MusicTrack {
    std::unique_ptr<eng::engine::SoundBuffer> buffer;
    std::unique_ptr<eng::engine::Sound> sound;
    std::string name;
    bool isLooping = true;
    float baseVolume = 70.0f;
};

/**
 * @brief Structure pour les effets sonores
 */
struct SoundEffect {
    std::unique_ptr<eng::engine::SoundBuffer> buffer;
    std::unique_ptr<eng::engine::Sound> sound;
    std::string name;
    float baseVolume = 100.0f;
};

/**
 * @brief Gestionnaire centralisé de l'audio du jeu
 * 
 * Cette classe gère :
 * - La musique de fond (menu, stages, boss)
 * - Les effets sonores
 * - Les transitions et fades
 * - Les volumes et paramètres audio
 * - Le contexte musical par état de jeu
 */
class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    // ========================================
    // INITIALISATION
    // ========================================
    
    /**
     * @brief Initialise le gestionnaire audio
     * @param assetBasePath Chemin de base pour les assets audio
     */
    bool Initialize(const std::string& assetBasePath);
    
    /**
     * @brief Charge la configuration audio depuis Lua
     * @param configPath Chemin vers le fichier de configuration
     */
    bool LoadAudioConfig(const std::string& configPath);

    // ========================================
    // MISE À JOUR
    // ========================================
    
    /**
     * @brief Met à jour le système audio (fades, transitions)
     * @param deltaTime Temps écoulé depuis la dernière frame
     */
    void Update(float deltaTime);

    // ========================================
    // MUSIQUE
    // ========================================
    
    /**
     * @brief Charge un track musical
     * @param name Nom du track
     * @param filePath Chemin vers le fichier audio
     * @param loop Si la musique doit être en boucle
     * @param volume Volume de base (0-100)
     */
    bool LoadMusic(const std::string& name, const std::string& filePath, bool loop = true, float volume = 70.0f);
    
    /**
     * @brief Joue une musique
     * @param name Nom du track à jouer
     * @param loop Force le mode boucle
     */
    void PlayMusic(const std::string& name, bool loop = true);
    
    /**
     * @brief Fade vers une nouvelle musique
     * @param name Nom du nouveau track
     * @param duration Durée de la transition en secondes
     */
    void FadeToMusic(const std::string& name, float duration = 2.0f);
    
    /**
     * @brief Arrête la musique actuelle
     */
    void StopMusic();
    
    /**
     * @brief Met en pause la musique
     */
    void PauseMusic();
    
    /**
     * @brief Reprend la musique
     */
    void ResumeMusic();
    
    /**
     * @brief Définit le volume global de la musique (0-100)
     */
    void SetMusicVolume(float volume);
    
    /**
     * @brief Obtient le volume global de la musique
     */
    float GetMusicVolume() const;

    // ========================================
    // EFFETS SONORES
    // ========================================
    
    /**
     * @brief Charge un effet sonore
     * @param name Nom de l'effet
     * @param filePath Chemin vers le fichier
     * @param volume Volume de base (0-100)
     */
    bool LoadSFX(const std::string& name, const std::string& filePath, float volume = 100.0f);
    
    /**
     * @brief Joue un effet sonore
     * @param name Nom de l'effet
     * @param volumeMultiplier Multiplicateur de volume (défaut: 1.0)
     */
    void PlaySFX(const std::string& name, float volumeMultiplier = 1.0f);
    
    /**
     * @brief Arrête tous les effets sonores
     */
    void StopAllSounds();
    
    /**
     * @brief Définit le volume global des SFX (0-100)
     */
    void SetSFXVolume(float volume);
    
    /**
     * @brief Obtient le volume global des SFX
     */
    float GetSFXVolume() const;

    // ========================================
    // CONTEXTE MUSICAL
    // ========================================
    
    /**
     * @brief Définit le stage actuel (change la musique automatiquement)
     */
    void SetCurrentStage(int stage);
    
    /**
     * @brief Appelé quand un boss apparaît
     */
    void OnBossSpawned();
    
    /**
     * @brief Appelé quand un boss est vaincu
     */
    void OnBossDefeated();
    
    /**
     * @brief Appelé en cas de game over
     */
    void OnGameOver();
    
    /**
     * @brief Appelé en cas de victoire
     */
    void OnVictory();

    // ========================================
    // SAUVEGARDE/CHARGEMENT
    // ========================================
    
    /**
     * @brief Charge les paramètres audio depuis un fichier
     */
    bool LoadUserSettings(const std::string& settingsPath);
    
    /**
     * @brief Sauvegarde les paramètres audio
     */
    bool SaveUserSettings(const std::string& settingsPath);

    // ========================================
    // DEBUG
    // ========================================
    
    /**
     * @brief Active/désactive les logs de debug
     */
    void SetDebugMode(bool enabled);
    
    /**
     * @brief Affiche l'état du système audio
     */
    void DebugPrintState() const;

private:
    // Tracks musicaux
    std::unordered_map<std::string, std::unique_ptr<MusicTrack>> musicTracks;
    std::unordered_map<std::string, std::unique_ptr<SoundEffect>> soundEffects;
    
    // État du système audio
    MusicTrack* currentMusic;
    std::string currentMusicName;
    float globalMusicVolume;
    float globalSFXVolume;
    
    // Système de fade
    bool isFading;
    bool fadeOutComplete;
    float fadeTimer;
    float fadeDuration;
    float fadeFromVolume;
    float fadeToVolume;
    std::string fadeToTrack;
    
    // Contexte de jeu
    int currentStage;
    bool bossMode;
    std::string assetBasePath;
    
    // Flags
    bool debugMode;
    bool initialized;
    
    // Méthodes privées
    void UpdateFade(float deltaTime);
    void ApplyVolumeToMusic();
    void ApplyVolumeToSFX();
    std::string GetStageMusic(int stage) const;
    std::string GetBossMusic(int stage) const;
    void InternalStopMusic();
    void InternalPlayMusic(const std::string& name);
};

} // namespace RType::Core
