#ifndef GAME_COMPONENTS_LASER_BEAM_HPP
#define GAME_COMPONENTS_LASER_BEAM_HPP

/**
 * @brief Component for continuous laser beam
 * 
 * Laser beams are different from regular projectiles:
 * - They extend across the screen
 * - They deal damage per tick (not on collision)
 * - They only exist while the fire button is held
 */
struct LaserBeam {
    float damagePerSecond = 50.0f;  // Damage dealt per second
    float width = 10.0f;             // Thickness of the beam
    float maxLength = 1500.0f;       // Maximum beam length
    bool active = true;              // Is the beam currently active?
};

#endif // GAME_COMPONENTS_LASER_BEAM_HPP
