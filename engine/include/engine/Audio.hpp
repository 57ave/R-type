#ifndef ENG_ENGINE_AUDIO_HPP
#define ENG_ENGINE_AUDIO_HPP

#include <string>
#include <memory>
#include <unordered_map>

namespace eng {
    namespace engine {

        // Forward declarations for SFML types (hidden from client)
        namespace internal {
            class SoundBufferImpl;
            class SoundImpl;
        }

        // Sound Buffer - Stores audio data
        class SoundBuffer {
        public:
            SoundBuffer();
            ~SoundBuffer();

            // Load from file
            bool loadFromFile(const std::string& filename);

            // Get the internal implementation (for engine use only)
            void* getInternalBuffer() const;

        private:
            std::unique_ptr<internal::SoundBufferImpl> m_impl;
        };

        // Sound - Plays audio from a buffer
        class Sound {
        public:
            Sound();
            ~Sound();

            // Set the buffer to play
            void setBuffer(const SoundBuffer& buffer);

            // Playback control
            void play();
            void pause();
            void stop();

            // Properties
            void setVolume(float volume); // 0-100
            void setLoop(bool loop);
            void setPitch(float pitch);

            float getVolume() const;
            bool getLoop() const;
            float getPitch() const;

            // Status
            enum Status {
                Stopped,
                Paused,
                Playing
            };
            Status getStatus() const;

        private:
            std::unique_ptr<internal::SoundImpl> m_impl;
        };

        // AudioManager - Manages all audio resources
        class AudioManager {
        public:
            AudioManager();
            ~AudioManager();

            // Load and cache sound buffers
            bool loadSound(const std::string& name, const std::string& filename);
            SoundBuffer* getSound(const std::string& name);

            // Play sound (creates a temporary Sound instance)
            void playSound(const std::string& name, float volume = 100.0f);

            // Global volume control
            void setGlobalVolume(float volume);
            float getGlobalVolume() const;

        private:
            std::unordered_map<std::string, std::unique_ptr<SoundBuffer>> m_soundBuffers;
            float m_globalVolume;
        };

    } // namespace engine
} // namespace eng

#endif // ENG_ENGINE_AUDIO_HPP
