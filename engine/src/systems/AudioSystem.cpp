// ============================================
// AudioSystem.cpp - ECS Audio System Implementation
// ============================================

#include <systems/AudioSystem.hpp>
#include "core/Logger.hpp"
#include <algorithm>

namespace eng {
namespace engine {
namespace systems {

void AudioSystem::Init(const std::string& sfxPath) {
    m_baseSFXPath = sfxPath;
    LOG_INFO("AUDIOSYSTEM", "Initialized with SFX path: " + m_baseSFXPath);
}

void AudioSystem::Update(float /*deltaTime*/) {
    if (!m_coordinator) return;

    // Process all entities with AudioSource component
    for (auto entity : mEntities) {
        if (!m_coordinator->HasComponent<AudioSource>(entity)) continue;
        
        auto& audioSrc = m_coordinator->GetComponent<AudioSource>(entity);
        
        // Check if sound should start playing
        if (audioSrc.playOnStart && !audioSrc.isPlaying) {
            // Get or load the sound buffer
            auto* buffer = GetOrLoadBuffer(audioSrc.soundPath);
            if (!buffer) {
                LOG_ERROR("AUDIOSYSTEM", "Failed to load sound: " + audioSrc.soundPath);
                audioSrc.playOnStart = false; // Don't try again
                continue;
            }

            // Create and play the sound
            auto sound = std::make_unique<eng::engine::Sound>();
            sound->setBuffer(*buffer);
            
            // Apply volume: component volume * global SFX volume
            float finalVolume = (audioSrc.volume / 100.0f) * m_globalSFXVolume;
            sound->setVolume(finalVolume);
            sound->setLoop(audioSrc.loop);
            sound->play();

            m_entitySounds[entity] = std::move(sound);
            audioSrc.isPlaying = true;
            
            LOG_INFO("AUDIOSYSTEM", "Playing: " + audioSrc.soundPath + " (Vol: " + std::to_string(finalVolume) + "%)");
        }

        // Check if sound has finished (for non-looping sounds)
        if (audioSrc.isPlaying && m_entitySounds.find(entity) != m_entitySounds.end()) {
            auto& sound = m_entitySounds[entity];
            if (sound->getStatus() == eng::engine::Sound::Stopped && !audioSrc.loop) {
                // Sound finished
                m_entitySounds.erase(entity);
                audioSrc.isPlaying = false;

                // Auto-destroy entity if SoundEffect component requests it
                if (m_coordinator->HasComponent<SoundEffect>(entity)) {
                    auto& sfx = m_coordinator->GetComponent<SoundEffect>(entity);
                    if (sfx.autoDestroy) {
                        m_coordinator->DestroyEntity(entity);
                    }
                }
            }
        }
    }

    // Cleanup finished one-shot sounds
    CleanupFinishedSounds();
}

void AudioSystem::Shutdown() {
    LOG_INFO("AUDIOSYSTEM", "Shutting down...");
    
    // Stop and clear all entity sounds
    for (auto& [entity, sound] : m_entitySounds) {
        sound->stop();
    }
    m_entitySounds.clear();
    
    // Stop and clear one-shot sounds
    for (auto& sound : m_activeSounds) {
        sound->stop();
    }
    m_activeSounds.clear();
    
    // Clear sound buffers
    m_soundBuffers.clear();
    
    LOG_INFO("AUDIOSYSTEM", "Shutdown complete");
}

void AudioSystem::SetSFXVolume(float volume) {
    m_globalSFXVolume = std::clamp(volume, 0.0f, 100.0f);
    
    // Update volume on all currently playing entity sounds
    for (auto& [entity, sound] : m_entitySounds) {
        if (m_coordinator && m_coordinator->HasComponent<AudioSource>(entity)) {
            auto& audioSrc = m_coordinator->GetComponent<AudioSource>(entity);
            float finalVolume = (audioSrc.volume / 100.0f) * m_globalSFXVolume;
            sound->setVolume(finalVolume);
        }
    }
    
    // Update volume on one-shot sounds
    for (auto& sound : m_activeSounds) {
        sound->setVolume(m_globalSFXVolume);
    }
    
    LOG_INFO("AUDIOSYSTEM", "Global SFX volume set to: " + std::to_string(m_globalSFXVolume) + "%");
}

void AudioSystem::PreloadSound(const std::string& name, const std::string& path) {
    if (m_soundBuffers.find(name) != m_soundBuffers.end()) {
        LOG_INFO("AUDIOSYSTEM", "Sound already preloaded: " + name);
        return;
    }

    auto buffer = std::make_unique<eng::engine::SoundBuffer>();
    if (buffer->loadFromFile(path)) {
        m_soundBuffers[name] = std::move(buffer);
        LOG_INFO("AUDIOSYSTEM", "Preloaded sound: " + name + " from " + path);
    } else {
        LOG_ERROR("AUDIOSYSTEM", "Failed to preload: " + path);
    }
}

void AudioSystem::PlaySFX(const std::string& name, float volumeMultiplier) {
    // Check if sound is preloaded
    if (m_soundBuffers.find(name) == m_soundBuffers.end()) {
        // Try to load from base path
        std::string fullPath = m_baseSFXPath + name;
        auto buffer = std::make_unique<eng::engine::SoundBuffer>();
        if (!buffer->loadFromFile(fullPath)) {
            LOG_WARNING("AUDIOSYSTEM", "Cannot play SFX, not found: " + name);
            return;
        }
        m_soundBuffers[name] = std::move(buffer);
    }

    // Create and play the sound
    auto sound = std::make_unique<eng::engine::Sound>();
    sound->setBuffer(*m_soundBuffers[name]);
    sound->setVolume(m_globalSFXVolume * volumeMultiplier);
    sound->setLoop(false);
    sound->play();

    m_activeSounds.push_back(std::move(sound));
    
    LOG_INFO("AUDIOSYSTEM", "SFX: " + name + " (Vol: " + std::to_string(m_globalSFXVolume * volumeMultiplier) + "%)");
}

void AudioSystem::StopAllSounds() {
    for (auto& [entity, sound] : m_entitySounds) {
        sound->stop();
    }
    m_entitySounds.clear();

    for (auto& sound : m_activeSounds) {
        sound->stop();
    }
    m_activeSounds.clear();
    
    LOG_INFO("AUDIOSYSTEM", "All sounds stopped");
}

SoundBuffer* AudioSystem::GetOrLoadBuffer(const std::string& soundPath) {
    // Check cache first
    if (m_soundBuffers.find(soundPath) != m_soundBuffers.end()) {
        return m_soundBuffers[soundPath].get();
    }

    // Try to load from full path
    std::string fullPath = m_baseSFXPath + soundPath;
    auto buffer = std::make_unique<SoundBuffer>();
    
    if (!buffer->loadFromFile(fullPath)) {
        // Try without base path (maybe it's already a full path)
        if (!buffer->loadFromFile(soundPath)) {
            return nullptr;
        }
    }

    auto* ptr = buffer.get();
    m_soundBuffers[soundPath] = std::move(buffer);
    return ptr;
}

void AudioSystem::CleanupFinishedSounds() {
    m_activeSounds.erase(
        std::remove_if(m_activeSounds.begin(), m_activeSounds.end(),
            [](const std::unique_ptr<Sound>& sound) {
                return sound->getStatus() == Sound::Stopped;
            }),
        m_activeSounds.end()
    );
}

} // namespace systems
} // namespace engine
} // namespace eng
