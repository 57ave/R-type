#ifndef COMPONENTS_SCORE_HPP
#define COMPONENTS_SCORE_HPP

#include <cstdint>

struct Score {
    uint32_t current = 0;
    uint32_t highScore = 0;
    
    void addPoints(uint32_t points) {
        current += points;
        if (current > highScore) {
            highScore = current;
        }
    }
    
    void reset() {
        current = 0;
    }
};

#endif // COMPONENTS_SCORE_HPP
