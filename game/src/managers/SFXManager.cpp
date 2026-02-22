/**
 * SFXManager.cpp - Sound effects manager implementation
 */

#include "managers/SFXManager.hpp"
#include "core/Logger.hpp"
#include <algorithm>

SFXManager::SFXManager()
    : sfxVolume_(80.0f)
    , masterVolume_(100.0f)
{
}

SFXManager::~SFXManager()
{
    stopAll();
}

void SFXManager::preload(const std::string& name, const std::string& filepath)
{
    if (buffers_.find(name) != buffers_.end()) {
        return; // Already loaded
    }

    auto buffer = std::make_unique<sf::SoundBuffer>();
    if (!buffer->loadFromFile(filepath)) {
        LOG_ERROR("SFXMANAGER", "Failed to load: " + filepath);
        return;
    }

    buffers_[name] = std::move(buffer);
    LOG_INFO("SFXMANAGER", "Preloaded: " + name + " (" + filepath + ")");
}

void SFXManager::play(const std::string& name, float volumeMul)
{
    auto it = buffers_.find(name);
    if (it == buffers_.end()) {
        LOG_WARNING("SFXMANAGER", "Sound not preloaded: " + name);
        return;
    }

    auto sound = std::make_unique<sf::Sound>();
    sound->setBuffer(*it->second);
    sound->setVolume(getEffectiveVolume() * volumeMul);
    sound->setLoop(false);
    sound->play();

    activeSounds_.push_back({std::move(sound), -1.0f});
}

void SFXManager::playPartial(const std::string& name, float durationSec, float volumeMul)
{
    auto it = buffers_.find(name);
    if (it == buffers_.end()) {
        LOG_WARNING("SFXMANAGER", "Sound not preloaded: " + name);
        return;
    }

    auto sound = std::make_unique<sf::Sound>();
    sound->setBuffer(*it->second);
    sound->setVolume(getEffectiveVolume() * volumeMul);
    sound->setLoop(false);
    sound->play();

    activeSounds_.push_back({std::move(sound), durationSec});
}

void SFXManager::update(float deltaTime)
{
    // Remove finished sounds and handle partial-duration sounds
    activeSounds_.erase(
        std::remove_if(activeSounds_.begin(), activeSounds_.end(),
            [deltaTime](ActiveSound& as) {
                // Check if partial duration expired
                if (as.remainingDuration >= 0.0f) {
                    as.remainingDuration -= deltaTime;
                    if (as.remainingDuration <= 0.0f) {
                        as.sound->stop();
                        return true;
                    }
                }
                // Check if sound naturally finished
                return as.sound->getStatus() == sf::Sound::Stopped;
            }),
        activeSounds_.end()
    );
}

void SFXManager::setSFXVolume(float volume)
{
    sfxVolume_ = std::max(0.0f, std::min(100.0f, volume));
    // Update volume on all active sounds
    for (auto& as : activeSounds_) {
        as.sound->setVolume(getEffectiveVolume());
    }
}

void SFXManager::setMasterVolume(float volume)
{
    masterVolume_ = std::max(0.0f, std::min(100.0f, volume));
    for (auto& as : activeSounds_) {
        as.sound->setVolume(getEffectiveVolume());
    }
}

void SFXManager::stopAll()
{
    for (auto& as : activeSounds_) {
        as.sound->stop();
    }
    activeSounds_.clear();
}

float SFXManager::getEffectiveVolume() const
{
    return (sfxVolume_ * masterVolume_) / 100.0f;
}
