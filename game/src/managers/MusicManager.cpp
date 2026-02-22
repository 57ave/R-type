/**
 * MusicManager.cpp - Music streaming manager implementation
 */

#include "managers/MusicManager.hpp"
#include "core/Logger.hpp"

MusicManager::MusicManager()
    : musicVolume_(70.0f)
    , masterVolume_(100.0f)
{
}

MusicManager::~MusicManager()
{
    stop();
}

void MusicManager::play(const std::string& filepath, bool loop)
{
    // If the same track is already playing, don't restart it
    if (currentTrack_ == filepath && isPlaying()) {
        return;
    }

    // Stop current music
    stop();

    // Create new music instance
    music_ = std::make_unique<sf::Music>();
    if (!music_->openFromFile(filepath)) {
        LOG_ERROR("MUSICMANAGER", "Failed to open music: " + filepath);
        music_.reset();
        currentTrack_.clear();
        return;
    }

    currentTrack_ = filepath;
    music_->setLoop(loop);
    applyVolume();
    music_->play();

    LOG_INFO("MUSICMANAGER", "Playing: " + filepath
              + " (loop=" + (loop ? "true" : "false")
              + ", vol=" + std::to_string(getEffectiveVolume()) + "%)");
}

void MusicManager::stop()
{
    if (music_) {
        music_->stop();
        music_.reset();
    }
    currentTrack_.clear();
}

void MusicManager::pause()
{
    if (music_ && music_->getStatus() == sf::Music::Playing) {
        music_->pause();
    }
}

void MusicManager::resume()
{
    if (music_ && music_->getStatus() == sf::Music::Paused) {
        music_->play();
    }
}

void MusicManager::setMusicVolume(float volume)
{
    musicVolume_ = std::max(0.0f, std::min(100.0f, volume));
    applyVolume();
}

void MusicManager::setMasterVolume(float volume)
{
    masterVolume_ = std::max(0.0f, std::min(100.0f, volume));
    applyVolume();
}

bool MusicManager::isPlaying() const
{
    return music_ && music_->getStatus() == sf::Music::Playing;
}

void MusicManager::applyVolume()
{
    if (music_) {
        music_->setVolume(getEffectiveVolume());
    }
}

float MusicManager::getEffectiveVolume() const
{
    return (musicVolume_ * masterVolume_) / 100.0f;
}
