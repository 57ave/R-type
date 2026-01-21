#include "core/AudioManager.hpp"
#include <iostream>
#include <filesystem>
#include <algorithm>

namespace RType::Core {

AudioManager::AudioManager()
    : currentMusic(nullptr)
    , globalMusicVolume(70.0f)
    , globalSFXVolume(100.0f)
    , isFading(false)
    , fadeTimer(0.0f)
    , fadeDuration(0.0f)
    , fadeFromVolume(0.0f)
    , fadeToVolume(0.0f)
    , currentStage(0)
    , bossMode(false)
    , debugMode(false)
    , initialized(false)
{
    std::cout << "[AudioManager] Created" << std::endl;
}

AudioManager::~AudioManager() {
    StopMusic();
    StopAllSounds();
    std::cout << "[AudioManager] Destroyed" << std::endl;
}

bool AudioManager::Initialize(const std::string& assetBasePath) {
    this->assetBasePath = assetBasePath;
    
    // Chargement des musiques par défaut
    std::string soundsPath = assetBasePath + "game/assets/sounds/";
    
    // Musique de menu
    if (!LoadMusic("menu", soundsPath + "Title.ogg", true, 70.0f)) {
        std::cerr << "[AudioManager] Warning: Could not load menu music" << std::endl;
    }
    
    // Musiques de stages (si disponibles)
    for (int i = 1; i <= 5; ++i) {
        std::string stagePath = soundsPath + "Stage" + std::to_string(i) + ".ogg";
        if (std::filesystem::exists(stagePath)) {
            LoadMusic("stage" + std::to_string(i), stagePath, true, 80.0f);
        }
    }
    
    // Musiques de boss (si disponibles)
    for (int i = 1; i <= 5; ++i) {
        std::string bossPath = soundsPath + "Boss" + std::to_string(i) + ".ogg";
        if (std::filesystem::exists(bossPath)) {
            LoadMusic("boss" + std::to_string(i), bossPath, true, 85.0f);
        }
    }
    
    // Musiques spéciales
    if (std::filesystem::exists(soundsPath + "GameOver.ogg")) {
        LoadMusic("gameover", soundsPath + "GameOver.ogg", false, 75.0f);
    }
    
    if (std::filesystem::exists(soundsPath + "Victory.ogg")) {
        LoadMusic("victory", soundsPath + "Victory.ogg", false, 80.0f);
    }
    
    // Chargement des SFX par défaut
    std::string vfxPath = assetBasePath + "game/assets/vfx/";
    
    if (!LoadSFX("shoot", vfxPath + "shoot.ogg", 80.0f)) {
        std::cerr << "[AudioManager] Warning: Could not load shoot sound" << std::endl;
    }
    
    if (!LoadSFX("explosion", vfxPath + "explosion.ogg", 90.0f)) {
        std::cerr << "[AudioManager] Warning: Could not load explosion sound" << std::endl;
    }
    
    initialized = true;
    std::cout << "[AudioManager] Initialized with " << musicTracks.size() 
              << " music tracks and " << soundEffects.size() << " sound effects" << std::endl;
    
    return true;
}

bool AudioManager::LoadAudioConfig(const std::string& configPath) {
    // TODO: Implémenter le chargement depuis Lua
    std::cout << "[AudioManager] Loading audio config from: " << configPath << std::endl;
    return true;
}

void AudioManager::Update(float deltaTime) {
    if (!initialized) return;
    
    UpdateFade(deltaTime);
}

bool AudioManager::LoadMusic(const std::string& name, const std::string& filePath, bool loop, float volume) {
    auto track = std::make_unique<MusicTrack>();
    track->buffer = std::make_unique<eng::engine::SoundBuffer>();
    
    if (!track->buffer->loadFromFile(filePath)) {
        std::cerr << "[AudioManager] Failed to load music: " << filePath << std::endl;
        return false;
    }
    
    track->sound = std::make_unique<eng::engine::Sound>();
    track->sound->setBuffer(*(track->buffer));
    track->sound->setLoop(loop);
    track->sound->setVolume(volume * (globalMusicVolume / 100.0f));
    track->name = name;
    track->isLooping = loop;
    track->baseVolume = volume;
    
    musicTracks[name] = std::move(track);
    
    if (debugMode) {
        std::cout << "[AudioManager] Loaded music: " << name << " (" << filePath << ")" << std::endl;
    }
    
    return true;
}

void AudioManager::PlayMusic(const std::string& name, bool loop) {
    InternalPlayMusic(name);
    
    if (currentMusic && loop != currentMusic->isLooping) {
        currentMusic->sound->setLoop(loop);
        currentMusic->isLooping = loop;
    }
}

void AudioManager::FadeToMusic(const std::string& name, float duration) {
    if (musicTracks.find(name) == musicTracks.end()) {
        std::cerr << "[AudioManager] Music track not found: " << name << std::endl;
        return;
    }
    
    fadeToTrack = name;
    fadeDuration = duration;
    fadeTimer = 0.0f;
    
    if (currentMusic) {
        isFading = true;
        fadeFromVolume = currentMusic->sound->getVolume();
        fadeToVolume = 0.0f;
        
        std::cout << "[AudioManager] Starting fade from " << currentMusicName 
                  << " to " << name << " (duration: " << duration << "s)" << std::endl;
    } else {
        // Pas de musique actuelle, jouer directement
        InternalPlayMusic(name);
    }
}

void AudioManager::StopMusic() {
    InternalStopMusic();
    isFading = false;
    currentMusicName.clear();
}

void AudioManager::PauseMusic() {
    if (currentMusic && currentMusic->sound) {
        currentMusic->sound->pause();
        
        if (debugMode) {
            std::cout << "[AudioManager] Music paused: " << currentMusicName << std::endl;
        }
    }
}

void AudioManager::ResumeMusic() {
    if (currentMusic && currentMusic->sound) {
        currentMusic->sound->play();
        
        if (debugMode) {
            std::cout << "[AudioManager] Music resumed: " << currentMusicName << std::endl;
        }
    }
}

void AudioManager::SetMusicVolume(float volume) {
    globalMusicVolume = std::clamp(volume, 0.0f, 100.0f);
    ApplyVolumeToMusic();
    
    if (debugMode) {
        std::cout << "[AudioManager] Music volume set to: " << globalMusicVolume << "%" << std::endl;
    }
}

float AudioManager::GetMusicVolume() const {
    return globalMusicVolume;
}

bool AudioManager::LoadSFX(const std::string& name, const std::string& filePath, float volume) {
    auto sfx = std::make_unique<SoundEffect>();
    sfx->buffer = std::make_unique<eng::engine::SoundBuffer>();
    
    if (!sfx->buffer->loadFromFile(filePath)) {
        if (debugMode) {
            std::cerr << "[AudioManager] Failed to load SFX: " << filePath << std::endl;
        }
        return false;
    }
    
    sfx->sound = std::make_unique<eng::engine::Sound>();
    sfx->sound->setBuffer(*(sfx->buffer));
    sfx->sound->setVolume(volume * (globalSFXVolume / 100.0f));
    sfx->name = name;
    sfx->baseVolume = volume;
    
    soundEffects[name] = std::move(sfx);
    
    if (debugMode) {
        std::cout << "[AudioManager] Loaded SFX: " << name << " (" << filePath << ")" << std::endl;
    }
    
    return true;
}

void AudioManager::PlaySFX(const std::string& name, float volumeMultiplier) {
    auto it = soundEffects.find(name);
    if (it == soundEffects.end()) {
        if (debugMode) {
            std::cerr << "[AudioManager] SFX not found: " << name << std::endl;
        }
        return;
    }
    
    auto& sfx = it->second;
    float finalVolume = sfx->baseVolume * volumeMultiplier * (globalSFXVolume / 100.0f);
    sfx->sound->setVolume(finalVolume);
    sfx->sound->play();
    
    if (debugMode) {
        std::cout << "[AudioManager] Playing SFX: " << name << " (volume: " << finalVolume << ")" << std::endl;
    }
}

void AudioManager::StopAllSounds() {
    for (auto& pair : soundEffects) {
        pair.second->sound->stop();
    }
    
    if (debugMode) {
        std::cout << "[AudioManager] All sounds stopped" << std::endl;
    }
}

void AudioManager::SetSFXVolume(float volume) {
    globalSFXVolume = std::clamp(volume, 0.0f, 100.0f);
    ApplyVolumeToSFX();
    
    if (debugMode) {
        std::cout << "[AudioManager] SFX volume set to: " << globalSFXVolume << "%" << std::endl;
    }
}

float AudioManager::GetSFXVolume() const {
    return globalSFXVolume;
}

void AudioManager::SetCurrentStage(int stage) {
    if (currentStage == stage) return;
    
    currentStage = stage;
    bossMode = false;
    
    std::string stageMusic = GetStageMusic(stage);
    if (!stageMusic.empty() && musicTracks.find(stageMusic) != musicTracks.end()) {
        FadeToMusic(stageMusic, 1.5f);
        std::cout << "[AudioManager] Stage " << stage << " - playing: " << stageMusic << std::endl;
    } else {
        std::cout << "[AudioManager] No music available for stage " << stage << std::endl;
    }
}

void AudioManager::OnBossSpawned() {
    if (bossMode) return;
    
    bossMode = true;
    std::string bossMusic = GetBossMusic(currentStage);
    
    if (!bossMusic.empty() && musicTracks.find(bossMusic) != musicTracks.end()) {
        FadeToMusic(bossMusic, 1.0f);
        std::cout << "[AudioManager] Boss spawned - playing: " << bossMusic << std::endl;
    } else {
        std::cout << "[AudioManager] No boss music available for stage " << currentStage << std::endl;
    }
}

void AudioManager::OnBossDefeated() {
    if (!bossMode) return;
    
    bossMode = false;
    
    // Retour à la musique de stage
    std::string stageMusic = GetStageMusic(currentStage);
    if (!stageMusic.empty()) {
        FadeToMusic(stageMusic, 2.0f);
        std::cout << "[AudioManager] Boss defeated - returning to: " << stageMusic << std::endl;
    }
}

void AudioManager::OnGameOver() {
    FadeToMusic("gameover", 1.0f);
    std::cout << "[AudioManager] Game Over" << std::endl;
}

void AudioManager::OnVictory() {
    FadeToMusic("victory", 1.5f);
    std::cout << "[AudioManager] Victory!" << std::endl;
}

bool AudioManager::LoadUserSettings(const std::string& settingsPath) {
    // TODO: Implémenter le chargement des paramètres
    std::cout << "[AudioManager] Loading user settings from: " << settingsPath << std::endl;
    return true;
}

bool AudioManager::SaveUserSettings(const std::string& settingsPath) {
    // TODO: Implémenter la sauvegarde des paramètres
    std::cout << "[AudioManager] Saving user settings to: " << settingsPath << std::endl;
    return true;
}

void AudioManager::SetDebugMode(bool enabled) {
    debugMode = enabled;
    std::cout << "[AudioManager] Debug mode: " << (enabled ? "enabled" : "disabled") << std::endl;
}

void AudioManager::DebugPrintState() const {
    std::cout << "[AudioManager] Current state:" << std::endl;
    std::cout << "  Music volume: " << globalMusicVolume << "%" << std::endl;
    std::cout << "  SFX volume: " << globalSFXVolume << "%" << std::endl;
    std::cout << "  Current music: " << (currentMusicName.empty() ? "none" : currentMusicName) << std::endl;
    std::cout << "  Stage: " << currentStage << std::endl;
    std::cout << "  Boss mode: " << (bossMode ? "true" : "false") << std::endl;
    std::cout << "  Fading: " << (isFading ? "true" : "false") << std::endl;
    std::cout << "  Loaded tracks: " << musicTracks.size() << std::endl;
    std::cout << "  Loaded SFX: " << soundEffects.size() << std::endl;
}

// ========================================
// MÉTHODES PRIVÉES
// ========================================

void AudioManager::UpdateFade(float deltaTime) {
    if (!isFading) return;
    
    fadeTimer += deltaTime;
    float progress = fadeTimer / fadeDuration;
    
    if (progress >= 1.0f) {
        // Fade terminé
        InternalStopMusic();
        InternalPlayMusic(fadeToTrack);
        isFading = false;
        fadeTimer = 0.0f;
    } else {
        // Fade en cours
        if (currentMusic) {
            float newVolume = fadeFromVolume * (1.0f - progress);
            currentMusic->sound->setVolume(newVolume);
        }
    }
}

void AudioManager::ApplyVolumeToMusic() {
    if (currentMusic) {
        float volume = currentMusic->baseVolume * (globalMusicVolume / 100.0f);
        currentMusic->sound->setVolume(volume);
    }
}

void AudioManager::ApplyVolumeToSFX() {
    for (auto& pair : soundEffects) {
        float volume = pair.second->baseVolume * (globalSFXVolume / 100.0f);
        pair.second->sound->setVolume(volume);
    }
}

std::string AudioManager::GetStageMusic(int stage) const {
    return "stage" + std::to_string(stage);
}

std::string AudioManager::GetBossMusic(int stage) const {
    return "boss" + std::to_string(stage);
}

void AudioManager::InternalStopMusic() {
    if (currentMusic && currentMusic->sound) {
        currentMusic->sound->stop();
        
        if (debugMode) {
            std::cout << "[AudioManager] Stopped music: " << currentMusicName << std::endl;
        }
    }
    
    currentMusic = nullptr;
}

void AudioManager::InternalPlayMusic(const std::string& name) {
    auto it = musicTracks.find(name);
    if (it == musicTracks.end()) {
        std::cerr << "[AudioManager] Music track not found: " << name << std::endl;
        return;
    }
    
    InternalStopMusic();
    
    currentMusic = it->second.get();
    currentMusicName = name;
    ApplyVolumeToMusic();
    currentMusic->sound->play();
    
    if (debugMode) {
        std::cout << "[AudioManager] Playing music: " << name << std::endl;
    }
}

} // namespace RType::Core
