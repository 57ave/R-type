/**
 * AudioManager.hpp - Audio Manager
 * 
 * Manages background music and sound effects for the game.
 */

#pragma once

#include <engine/Audio.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class AudioManager
{
public:
    AudioManager();
    ~AudioManager();

    /**
     * Initialize the audio manager and load all sounds
     * @param assetsBasePath Base path to assets directory
     * @return true if initialization succeeded
     */
    bool initialize(const std::string& assetsBasePath);

    /**
     * Update audio manager (for fading, etc.)
     * @param deltaTime Time elapsed since last update
     */
    void update(float deltaTime);

    // ========== MUSIC ==========

    /**
     * Play background music
     * @param musicName Name of the music to play (without path/extension)
     * @param loop Whether to loop the music
     * @param fadeIn Fade in duration in seconds (0 = no fade)
     */
    void playMusic(const std::string& musicName, bool loop = true, float fadeIn = 0.0f);

    /**
     * Stop currently playing music
     * @param fadeOut Fade out duration in seconds (0 = immediate stop)
     */
    void stopMusic(float fadeOut = 0.0f);

    /**
     * Pause music
     */
    void pauseMusic();

    /**
     * Resume music
     */
    void resumeMusic();

    /**
     * Set music volume (0.0 to 100.0)
     */
    void setMusicVolume(float volume);

    /**
     * Get music volume
     */
    float getMusicVolume() const { return musicVolume_; }

    // ========== SOUND EFFECTS ==========

    /**
     * Play a sound effect
     * @param sfxName Name of the sound effect (without path/extension)
     * @param volume Volume multiplier (0.0 to 1.0)
     */
    void playSFX(const std::string& sfxName, float volume = 1.0f);

    /**
     * Set global SFX volume (0.0 to 100.0)
     */
    void setSFXVolume(float volume);

    /**
     * Get global SFX volume
     */
    float getSFXVolume() const { return sfxVolume_; }

    /**
     * Stop all currently playing sound effects
     */
    void stopAllSFX();

private:
    // Music management
    std::unordered_map<std::string, std::unique_ptr<eng::engine::SoundBuffer>> musicBuffers_;
    std::unique_ptr<eng::engine::Sound> musicSound_;
    std::string currentMusic_;
    float musicVolume_;
    bool musicLooping_;

    // Fade management
    bool isFadingOut_;
    bool isFadingIn_;
    float fadeTimer_;
    float fadeDuration_;
    float targetVolume_;
    std::string nextMusic_;

    // SFX management
    std::unordered_map<std::string, std::unique_ptr<eng::engine::SoundBuffer>> sfxBuffers_;
    std::vector<std::unique_ptr<eng::engine::Sound>> activeSounds_;
    float sfxVolume_;

    // Paths
    std::string soundsPath_;
    std::string vfxPath_;

    // Helper methods
    bool loadMusicFile(const std::string& musicName);
    bool loadSFXFile(const std::string& sfxName);
    void cleanupFinishedSounds();
};
