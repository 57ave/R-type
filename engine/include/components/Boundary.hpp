#ifndef ENG_ENGINE_COMPONENTS_BOUNDARY_HPP
#define ENG_ENGINE_COMPONENTS_BOUNDARY_HPP

struct Boundary {
    bool destroyOutOfBounds = true;
    float margin = 100.0f;         // Extra margin before destruction

    // Optional: clamp to bounds instead of destroy
    bool clampToBounds = false;
};

#endif // ENG_ENGINE_COMPONENTS_BOUNDARY_HPP
