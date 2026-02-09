#ifndef ENG_ENGINE_COMPONENTS_AUDIOSOURCE_HPP
#define ENG_ENGINE_COMPONENTS_AUDIOSOURCE_HPP

#include <string>

struct AudioSource {
    std::string soundPath = "";
    float volume = 100.0f;         // 0-100
    bool loop = false;
    bool playOnStart = false;
    bool isPlaying = false;

    // Internal state (managed by AudioSystem)
    void* soundBuffer = nullptr;   // Opaque pointer to sound buffer
    void* sound = nullptr;          // Opaque pointer to sound instance
};

struct SoundEffect {
    bool autoDestroy = true;       // Destroy entity when sound finishes
};

#endif // ENG_ENGINE_COMPONENTS_AUDIOSOURCE_HPP
