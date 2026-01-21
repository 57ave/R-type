// ============================================
// AudioSystem.cpp - ECS Audio System Implementation
// ============================================

#include <algorithm>
#include <iostream>
#include <systems/AudioSystem.hpp>

namespace eng {
namespace engine {
namespace systems {

void AudioSystem::Init(const std::string& sfxPath) {
    m_baseSFXPath = sfxPath;
    std::cout << "[AudioSystem] Initialized with SFX path: " << m_baseSFXPath << std::endl;
}

void AudioSystem::Update(float /*deltaTime*/) {
    if (!m_coordinator)
        return;

    // Process all entities with AudioSource component
    for (auto entity : mEntities) {
        if (!m_coordinator->HasComponent<AudioSource>(entity))
            continue;

        auto& audioSrc = m_coordinator->GetComponent<AudioSource>(entity);

        // Check if sound should start playing
        if (audioSrc.playOnStart && !audioSrc.isPlaying) {
            // Get or load the sound buffer
            auto* buffer = GetOrLoadBuffer(audioSrc.soundPath);
            if (!buffer) {
                std::cerr << "[AudioSystem] Failed to load sound: " << audioSrc.soundPath
                          << std::endl;
                audioSrc.playOnStart = false;  // Don't try again
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

            std::cout << "[AudioSystem] Playing: " << audioSrc.soundPath << " (Vol: " << finalVolume
                      << "%)" << std::endl;
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
    std::cout << "[AudioSystem] Shutting down..." << std::endl;

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

    std::cout << "[AudioSystem] Shutdown complete" << std::endl;
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

    std::cout << "[AudioSystem] Global SFX volume set to: " << m_globalSFXVolume << "%"
              << std::endl;
}

void AudioSystem::PreloadSound(const std::string& name, const std::string& path) {
    if (m_soundBuffers.find(name) != m_soundBuffers.end()) {
        std::cout << "[AudioSystem] Sound already preloaded: " << name << std::endl;
        return;
    }

    auto buffer = std::make_unique<eng::engine::SoundBuffer>();
    if (buffer->loadFromFile(path)) {
        m_soundBuffers[name] = std::move(buffer);
        std::cout << "[AudioSystem] Preloaded sound: " << name << " from " << path << std::endl;
    } else {
        std::cerr << "[AudioSystem] Failed to preload: " << path << std::endl;
    }
}

void AudioSystem::PlaySFX(const std::string& name, float volumeMultiplier) {
    // Check if sound is preloaded
    if (m_soundBuffers.find(name) == m_soundBuffers.end()) {
        // Try to load from base path
        std::string fullPath = m_baseSFXPath + name;
        auto buffer = std::make_unique<eng::engine::SoundBuffer>();
        if (!buffer->loadFromFile(fullPath)) {
            std::cerr << "[AudioSystem] Cannot play SFX, not found: " << name << std::endl;
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

    std::cout << "[AudioSystem] SFX: " << name
              << " (Vol: " << (m_globalSFXVolume * volumeMultiplier) << "%)" << std::endl;
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

    std::cout << "[AudioSystem] All sounds stopped" << std::endl;
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
    m_activeSounds.erase(std::remove_if(m_activeSounds.begin(), m_activeSounds.end(),
                                        [](const std::unique_ptr<Sound>& sound) {
                                            return sound->getStatus() == Sound::Stopped;
                                        }),
                         m_activeSounds.end());
}

}  // namespace systems
}  // namespace engine
}  // namespace eng
