// ============================================
// AudioSystem.hpp - ECS Audio System for SFX
// ============================================
// Manages audio playback for entities with AudioSource component.
// Supports preloading, volume control, and automatic cleanup.
// ============================================

#ifndef ENG_ENGINE_SYSTEMS_AUDIOSYSTEM_HPP
#define ENG_ENGINE_SYSTEMS_AUDIOSYSTEM_HPP

#include <core/Export.hpp>
#include "ecs/System.hpp"
#include "ecs/Coordinator.hpp"
#include "ecs/Types.hpp"
#include "components/AudioSource.hpp"
#include "engine/Audio.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace eng {
namespace engine {
namespace systems {

class RTYPE_API AudioSystem : public ::ECS::System {
public:
    AudioSystem() = default;
    ~AudioSystem() override = default;

    // Initialize with base path for SFX files
    void Init(const std::string& sfxPath);
    void Update(float deltaTime) override;
    void Shutdown() override;

    void SetCoordinator(::ECS::Coordinator* coordinator) { m_coordinator = coordinator; }

    // Volume control
    void SetSFXVolume(float volume);
    float GetSFXVolume() const { return m_globalSFXVolume; }

    // Preload a sound effect (name is used as key for PlaySFX)
    void PreloadSound(const std::string& name, const std::string& path);

    // Play a preloaded or auto-loaded sound effect
    void PlaySFX(const std::string& name, float volumeMultiplier = 1.0f);

    // Stop all currently playing sounds
    void StopAllSounds();

private:
    ::ECS::Coordinator* m_coordinator = nullptr;
    
    // Base path for SFX files
    std::string m_baseSFXPath;
    
    // Global SFX volume (0-100)
    float m_globalSFXVolume = 100.0f;
    
    // Sound buffer cache
    std::map<std::string, std::unique_ptr<eng::engine::SoundBuffer>> m_soundBuffers;
    
    // Active sounds (for one-shot sounds)
    std::vector<std::unique_ptr<eng::engine::Sound>> m_activeSounds;
    
    // Entity-specific sounds
    std::map<::ECS::Entity, std::unique_ptr<eng::engine::Sound>> m_entitySounds;
    
    // Helper to get or load sound buffer
    eng::engine::SoundBuffer* GetOrLoadBuffer(const std::string& soundPath);
    
    // Clean up finished sounds
    void CleanupFinishedSounds();
};

} // namespace systems
} // namespace engine
} // namespace eng

#endif // ENG_ENGINE_SYSTEMS_AUDIOSYSTEM_HPP