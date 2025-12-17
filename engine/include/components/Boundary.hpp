#ifndef RTYPE_ENGINE_COMPONENTS_BOUNDARY_HPP
#define RTYPE_ENGINE_COMPONENTS_BOUNDARY_HPP

struct Boundary {
    bool destroyOutOfBounds = true;
    float margin = 100.0f;         // Extra margin before destruction

    // Optional: clamp to bounds instead of destroy
    bool clampToBounds = false;
};

#endif // RTYPE_ENGINE_COMPONENTS_BOUNDARY_HPP
