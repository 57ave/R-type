/**
 * AudioManager.cpp - Audio Manager Implementation
 */

#include "managers/AudioManager.hpp"
#include <iostream>
#include <algorithm>

AudioManager::AudioManager()
    : musicVolume_(70.0f)
    , musicLooping_(false)
    , isFadingOut_(false)
    , isFadingIn_(false)
    , fadeTimer_(0.0f)
    , fadeDuration_(0.0f)
    , targetVolume_(0.0f)
    , sfxVolume_(100.0f)
{
    musicSound_ = std::make_unique<eng::engine::Sound>();
}

AudioManager::~AudioManager()
{
    stopMusic();
    stopAllSFX();
}

bool AudioManager::initialize(const std::string& assetsBasePath)
{
    soundsPath_ = assetsBasePath + "/sounds/";
    vfxPath_ = assetsBasePath + "/vfx/";
    
    std::cout << "[AudioManager] ==================================" << std::endl;
    std::cout << "[AudioManager] Initialized with:" << std::endl;
    std::cout << "[AudioManager]   Assets base: " << assetsBasePath << std::endl;
    std::cout << "[AudioManager]   Sounds path: " << soundsPath_ << std::endl;
    std::cout << "[AudioManager]   VFX path: " << vfxPath_ << std::endl;
    std::cout << "[AudioManager] ==================================" << std::endl;
    
    return true;
}

void AudioManager::update(float deltaTime)
{
    // Debug: Check music status every second (roughly)
    static float debugTimer = 0.0f;
    debugTimer += deltaTime;
    if (debugTimer >= 1.0f && musicSound_) {
        auto status = musicSound_->getStatus();
        if (!currentMusic_.empty()) {
            std::cout << "[AudioManager] Music status: " << (int)status 
                      << " (Playing: " << currentMusic_ << ", Volume: " << musicSound_->getVolume() << ")" << std::endl;
        }
        debugTimer = 0.0f;
    }
    
    // Handle fade out
    if (isFadingOut_) {
        fadeTimer_ += deltaTime;
        float progress = std::min(fadeTimer_ / fadeDuration_, 1.0f);
        float currentVolume = musicVolume_ * (1.0f - progress);
        
        if (musicSound_) {
            musicSound_->setVolume(currentVolume);
        }
        
        if (progress >= 1.0f) {
            isFadingOut_ = false;
            if (musicSound_) {
                musicSound_->stop();
            }
            
            // If there's a next music queued, start it
            if (!nextMusic_.empty()) {
                playMusic(nextMusic_, musicLooping_, fadeDuration_);
                nextMusic_.clear();
            }
        }
    }
    
    // Handle fade in
    if (isFadingIn_) {
        fadeTimer_ += deltaTime;
        float progress = std::min(fadeTimer_ / fadeDuration_, 1.0f);
        float currentVolume = targetVolume_ * progress;
        
        if (musicSound_) {
            musicSound_->setVolume(currentVolume);
        }
        
        if (progress >= 1.0f) {
            isFadingIn_ = false;
        }
    }
    
    // Cleanup finished sound effects
    cleanupFinishedSounds();
}

void AudioManager::playMusic(const std::string& musicName, bool loop, float fadeIn)
{
    std::cout << "[AudioManager] ========================================" << std::endl;
    std::cout << "[AudioManager] playMusic() called:" << std::endl;
    std::cout << "[AudioManager]   Music name: " << musicName << std::endl;
    std::cout << "[AudioManager]   Loop: " << (loop ? "true" : "false") << std::endl;
    std::cout << "[AudioManager]   Fade in: " << fadeIn << "s" << std::endl;
    std::cout << "[AudioManager]   Current music: " << currentMusic_ << std::endl;
    std::cout << "[AudioManager]   musicSound_ ptr: " << (musicSound_ ? "valid" : "NULL") << std::endl;
    std::cout << "[AudioManager] ========================================" << std::endl;
    
    // If already playing this music, don't restart
    if (currentMusic_ == musicName && musicSound_ && musicSound_->getStatus() == eng::engine::Sound::Status::Playing) {
        std::cout << "[AudioManager] Already playing this music, skipping." << std::endl;
        return;
    }
    
    // Stop current music if any
    bool isCurrentlyPlaying = musicSound_ && musicSound_->getStatus() == eng::engine::Sound::Status::Playing;
    
    if (isCurrentlyPlaying && fadeIn > 0.0f) {
        // Fade out current, then fade in new
        std::cout << "[AudioManager] Fading out current music before starting new one..." << std::endl;
        nextMusic_ = musicName;
        musicLooping_ = loop;
        stopMusic(fadeIn);
        return;
    } else {
        // Stop immediately if no fade needed
        if (isCurrentlyPlaying) {
            std::cout << "[AudioManager] Stopping current music immediately..." << std::endl;
        }
        stopMusic();
    }
    
    // Load the music if not already loaded
    if (!loadMusicFile(musicName)) {
        std::cerr << "[AudioManager] Failed to play music: " << musicName << std::endl;
        return;
    }
    
    // Set the buffer and play
    auto it = musicBuffers_.find(musicName);
    if (it != musicBuffers_.end()) {
        std::cout << "[AudioManager] Found buffer for: " << musicName << std::endl;
        
        musicSound_->setBuffer(*it->second);
        musicSound_->setLoop(loop);
        
        if (fadeIn > 0.0f) {
            musicSound_->setVolume(0.0f);
            isFadingIn_ = true;
            fadeTimer_ = 0.0f;
            fadeDuration_ = fadeIn;
            targetVolume_ = musicVolume_;
            std::cout << "[AudioManager] Music will fade in from 0 to " << musicVolume_ << std::endl;
        } else {
            musicSound_->setVolume(musicVolume_);
            std::cout << "[AudioManager] Music volume set to: " << musicVolume_ << std::endl;
        }
        
        musicSound_->play();
        
        // Check if music is actually playing
        auto status = musicSound_->getStatus();
        std::cout << "[AudioManager] Music status after play: " << (int)status << " (0=Stopped, 1=Paused, 2=Playing)" << std::endl;
        
        currentMusic_ = musicName;
        musicLooping_ = loop;
        
        std::cout << "[AudioManager] Playing music: " << musicName << " (loop: " << loop << ", volume: " << musicVolume_ << ")" << std::endl;
    } else {
        std::cerr << "[AudioManager] ERROR: Buffer not found in map for: " << musicName << std::endl;
    }
}

void AudioManager::stopMusic(float fadeOut)
{
    if (!musicSound_) {
        return;
    }
    
    if (fadeOut > 0.0f && musicSound_->getStatus() == eng::engine::Sound::Status::Playing) {
        isFadingOut_ = true;
        fadeTimer_ = 0.0f;
        fadeDuration_ = fadeOut;
    } else {
        musicSound_->stop();
        currentMusic_.clear();
    }
}

void AudioManager::pauseMusic()
{
    if (musicSound_) {
        musicSound_->pause();
    }
}

void AudioManager::resumeMusic()
{
    if (musicSound_) {
        musicSound_->play();
    }
}

void AudioManager::setMusicVolume(float volume)
{
    musicVolume_ = std::clamp(volume, 0.0f, 100.0f);
    
    if (musicSound_ && !isFadingIn_ && !isFadingOut_) {
        musicSound_->setVolume(musicVolume_);
    }
}

void AudioManager::playSFX(const std::string& sfxName, float volume)
{
    // Load the SFX if not already loaded
    if (!loadSFXFile(sfxName)) {
        std::cerr << "[AudioManager] Failed to play SFX: " << sfxName << std::endl;
        return;
    }
    
    // Get the buffer
    auto it = sfxBuffers_.find(sfxName);
    if (it == sfxBuffers_.end()) {
        return;
    }
    
    // Create a new sound instance
    auto sound = std::make_unique<eng::engine::Sound>();
    sound->setBuffer(*it->second);
    sound->setVolume(sfxVolume_ * volume);
    sound->play();
    
    // Add to active sounds
    activeSounds_.push_back(std::move(sound));
}

void AudioManager::setSFXVolume(float volume)
{
    sfxVolume_ = std::clamp(volume, 0.0f, 100.0f);
}

void AudioManager::stopAllSFX()
{
    for (auto& sound : activeSounds_) {
        if (sound) {
            sound->stop();
        }
    }
    activeSounds_.clear();
}

bool AudioManager::loadMusicFile(const std::string& musicName)
{
    // Check if already loaded
    if (musicBuffers_.find(musicName) != musicBuffers_.end()) {
        std::cout << "[AudioManager] Music already loaded: " << musicName << std::endl;
        return true;
    }
    
    // Try to load the file
    std::string filePath = soundsPath_ + musicName + ".ogg";
    auto buffer = std::make_unique<eng::engine::SoundBuffer>();
    
    std::cout << "[AudioManager] Attempting to load music from: " << filePath << std::endl;
    
    if (!buffer->loadFromFile(filePath)) {
        // Try without adding .ogg (in case the name already has it)
        filePath = soundsPath_ + musicName;
        std::cout << "[AudioManager] First attempt failed, trying: " << filePath << std::endl;
        if (!buffer->loadFromFile(filePath)) {
            std::cerr << "[AudioManager] Failed to load music: " << filePath << std::endl;
            return false;
        }
    }
    
    musicBuffers_[musicName] = std::move(buffer);
    std::cout << "[AudioManager] Successfully loaded music: " << musicName << " from " << filePath << std::endl;
    return true;
}

bool AudioManager::loadSFXFile(const std::string& sfxName)
{
    // Check if already loaded
    if (sfxBuffers_.find(sfxName) != sfxBuffers_.end()) {
        return true;
    }
    
    // Try loading from vfx folder first
    std::string filePath = vfxPath_ + sfxName + ".ogg";
    auto buffer = std::make_unique<eng::engine::SoundBuffer>();
    
    if (!buffer->loadFromFile(filePath)) {
        // Try without adding .ogg
        filePath = vfxPath_ + sfxName;
        if (!buffer->loadFromFile(filePath)) {
            // Try sounds folder as fallback
            filePath = soundsPath_ + sfxName + ".ogg";
            if (!buffer->loadFromFile(filePath)) {
                filePath = soundsPath_ + sfxName;
                if (!buffer->loadFromFile(filePath)) {
                    std::cerr << "[AudioManager] Failed to load SFX: " << sfxName << std::endl;
                    return false;
                }
            }
        }
    }
    
    sfxBuffers_[sfxName] = std::move(buffer);
    std::cout << "[AudioManager] Loaded SFX: " << sfxName << std::endl;
    return true;
}

void AudioManager::cleanupFinishedSounds()
{
    activeSounds_.erase(
        std::remove_if(activeSounds_.begin(), activeSounds_.end(),
            [](const std::unique_ptr<eng::engine::Sound>& sound) {
                return sound->getStatus() == eng::engine::Sound::Status::Stopped;
            }),
        activeSounds_.end()
    );
}
