/**
 * SFXManager.hpp - Sound effects manager
 *
 * Manages short sound effects (shooting, damage, etc.) using sf::Sound/sf::SoundBuffer.
 * Supports preloading, volume control, and concurrent playback of multiple sounds.
 */

#pragma once

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

class SFXManager
{
public:
    SFXManager();
    ~SFXManager();

    /**
     * Preload a sound effect into memory.
     * @param name    Logical name (e.g. "shoot", "damage")
     * @param filepath Path to the sound file
     */
    void preload(const std::string& name, const std::string& filepath);

    /**
     * Play a preloaded sound effect.
     * @param name     Logical name of the sound
     * @param volumeMul Volume multiplier (0-1), applied on top of global sfx volume
     */
    void play(const std::string& name, float volumeMul = 1.0f);

    /**
     * Play a preloaded sound but only the first N seconds.
     * Useful for using only the beginning of a longer sound.
     * Stops the sound after `durationSec` seconds (checked in update()).
     */
    void playPartial(const std::string& name, float durationSec, float volumeMul = 1.0f);

    /**
     * Must be called each frame to clean up finished sounds
     * and handle partial-duration sounds.
     */
    void update(float deltaTime);

    /**
     * Set the SFX volume (0-100).
     */
    void setSFXVolume(float volume);
    float getSFXVolume() const { return sfxVolume_; }

    /**
     * Set the master volume (0-100). Shared with MusicManager.
     */
    void setMasterVolume(float volume);
    float getMasterVolume() const { return masterVolume_; }

    /**
     * Stop all currently playing sounds.
     */
    void stopAll();

private:
    struct ActiveSound {
        std::unique_ptr<sf::Sound> sound;
        float remainingDuration; // -1 = no limit (play to end)
    };

    std::unordered_map<std::string, std::unique_ptr<sf::SoundBuffer>> buffers_;
    std::vector<ActiveSound> activeSounds_;
    float sfxVolume_;    // 0-100
    float masterVolume_; // 0-100

    float getEffectiveVolume() const;
};
