#ifndef GAME_COMPONENTS_WAVE_MOTION_HPP
#define GAME_COMPONENTS_WAVE_MOTION_HPP

/**
 * @brief Component for projectiles with sinusoidal (wave) motion
 * 
 * Makes projectiles move in a wave pattern (up and down)
 */
struct WaveMotion {
    float frequency = 3.0f;      // How fast the wave oscillates (Hz)
    float amplitude = 50.0f;     // How far up/down the wave goes (pixels)
    float time = 0.0f;           // Current time for sine calculation
    float baseVelocityY = 0.0f;  // Store original Y velocity (unused, kept for compat)
    float baseY = 0.0f;          // Base Y position at spawn (position-based wave)
};

#endif // GAME_COMPONENTS_WAVE_MOTION_HPP
