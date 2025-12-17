/*
** EPITECH PROJECT, 2025
** Audio
** File description:
** Audio abstraction implementation with SFML backend
*/

#include "engine/Audio.hpp"
#include <SFML/Audio.hpp>
#include <iostream>

namespace rtype {
    namespace engine {
        namespace internal {

            // Internal SFML implementation classes
            class SoundBufferImpl {
            public:
                sf::SoundBuffer buffer;
            };

            class SoundImpl {
            public:
                sf::Sound sound;
            };

        } // namespace internal

        // ===== SoundBuffer Implementation =====

        SoundBuffer::SoundBuffer() 
            : m_impl(std::make_unique<internal::SoundBufferImpl>()) {
        }

        SoundBuffer::~SoundBuffer() = default;

        bool SoundBuffer::loadFromFile(const std::string& filename) {
            return m_impl->buffer.loadFromFile(filename);
        }

        void* SoundBuffer::getInternalBuffer() const {
            return (void*)&m_impl->buffer;
        }

        // ===== Sound Implementation =====

        Sound::Sound() 
            : m_impl(std::make_unique<internal::SoundImpl>()) {
        }

        Sound::~Sound() = default;

        void Sound::setBuffer(const SoundBuffer& buffer) {
            const sf::SoundBuffer* sfBuffer = static_cast<const sf::SoundBuffer*>(buffer.getInternalBuffer());
            if (sfBuffer) {
                m_impl->sound.setBuffer(*sfBuffer);
            }
        }

        void Sound::play() {
            m_impl->sound.play();
        }

        void Sound::pause() {
            m_impl->sound.pause();
        }

        void Sound::stop() {
            m_impl->sound.stop();
        }

        void Sound::setVolume(float volume) {
            m_impl->sound.setVolume(volume);
        }

        void Sound::setLoop(bool loop) {
            m_impl->sound.setLoop(loop);
        }

        void Sound::setPitch(float pitch) {
            m_impl->sound.setPitch(pitch);
        }

        float Sound::getVolume() const {
            return m_impl->sound.getVolume();
        }

        bool Sound::getLoop() const {
            return m_impl->sound.getLoop();
        }

        float Sound::getPitch() const {
            return m_impl->sound.getPitch();
        }

        Sound::Status Sound::getStatus() const {
            sf::Sound::Status sfStatus = m_impl->sound.getStatus();
            switch (sfStatus) {
                case sf::Sound::Stopped: return Status::Stopped;
                case sf::Sound::Paused: return Status::Paused;
                case sf::Sound::Playing: return Status::Playing;
                default: return Status::Stopped;
            }
        }

        // ===== AudioManager Implementation =====

        AudioManager::AudioManager() 
            : m_globalVolume(100.0f) {
        }

        AudioManager::~AudioManager() = default;

        bool AudioManager::loadSound(const std::string& name, const std::string& filename) {
            auto buffer = std::make_unique<SoundBuffer>();
            if (buffer->loadFromFile(filename)) {
                m_soundBuffers[name] = std::move(buffer);
                return true;
            }
            std::cerr << "Warning: Could not load sound: " << filename << std::endl;
            return false;
        }

        SoundBuffer* AudioManager::getSound(const std::string& name) {
            auto it = m_soundBuffers.find(name);
            if (it != m_soundBuffers.end()) {
                return it->second.get();
            }
            return nullptr;
        }

        void AudioManager::playSound(const std::string& name, float volume) {
            SoundBuffer* buffer = getSound(name);
            if (buffer) {
                // Create a temporary sound to play
                // Note: In a real implementation, you'd want to manage a pool of Sound instances
                // to avoid recreation on every play
                static Sound tempSound;
                tempSound.setBuffer(*buffer);
                tempSound.setVolume(volume * m_globalVolume / 100.0f);
                tempSound.play();
            }
        }

        void AudioManager::setGlobalVolume(float volume) {
            m_globalVolume = volume;
        }

        float AudioManager::getGlobalVolume() const {
            return m_globalVolume;
        }

    } // namespace engine
} // namespace rtype
