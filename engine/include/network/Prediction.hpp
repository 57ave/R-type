#pragma once

#include <deque>

#include "RTypeProtocol.hpp"

namespace Network {

class Prediction {
public:
    // Linear Interpolation for Entity State
    static EntityState interpolate(const EntityState& start, const EntityState& end, float t) {
        EntityState result = start;
        if (t <= 0.0f)
            return start;
        if (t >= 1.0f)
            return end;

        // Simple Lerp for positions
        result.x = static_cast<int16_t>(start.x + (end.x - start.x) * t);
        result.y = static_cast<int16_t>(start.y + (end.y - start.y) * t);

        // Velocities usually don't need interpolation for rendering, just position
        // But if we do:
        result.vx = static_cast<int16_t>(start.vx + (end.vx - start.vx) * t);
        result.vy = static_cast<int16_t>(start.vy + (end.vy - start.vy) * t);

        return result;
    }

    // Predict position based on velocity and delta time
    static void predict(EntityState& state, float dtSeconds) {
        // x is pixels, vx is pixels/sec (assumption)
        // But x is int16. We need to be careful with precision if dt is small.
        // Using float accumulator locally would be better in real engine.
        state.x += static_cast<int16_t>(state.vx * dtSeconds);
        state.y += static_cast<int16_t>(state.vy * dtSeconds);
    }
};

}  // namespace Network
