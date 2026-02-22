/**
 * MusicManager.hpp - Music streaming manager
 *
 * Uses SFML's sf::Music for streaming large .ogg files (BGM).
 * Supports volume control, looping, and crossfade-like transitions.
 */

#pragma once

#include <SFML/Audio/Music.hpp>
#include <string>
#include <memory>

class MusicManager
{
public:
    MusicManager();
    ~MusicManager();

    /**
     * Play a music file (streaming, not loaded into memory).
     * Stops the currently playing music first.
     * @param filepath Path to the music file (e.g. "assets/sounds/Title.ogg")
     * @param loop     Whether to loop the music
     */
    void play(const std::string& filepath, bool loop = true);

    /**
     * Stop the currently playing music.
     */
    void stop();

    /**
     * Pause the currently playing music.
     */
    void pause();

    /**
     * Resume a paused music.
     */
    void resume();

    /**
     * Set the music volume (0-100).
     * The actual volume applied is musicVolume * masterVolume / 100.
     */
    void setMusicVolume(float volume);
    float getMusicVolume() const { return musicVolume_; }

    /**
     * Set the master volume (0-100). Affects both music and SFX.
     */
    void setMasterVolume(float volume);
    float getMasterVolume() const { return masterVolume_; }

    /**
     * Check if music is currently playing.
     */
    bool isPlaying() const;

    /**
     * Get the currently playing music filepath.
     */
    const std::string& getCurrentTrack() const { return currentTrack_; }

    /**
     * Update volumes on currently playing music (call after volume change).
     */
    void applyVolume();

private:
    std::unique_ptr<sf::Music> music_;
    std::string currentTrack_;
    float musicVolume_;    // 0-100
    float masterVolume_;   // 0-100

    float getEffectiveVolume() const;
};
