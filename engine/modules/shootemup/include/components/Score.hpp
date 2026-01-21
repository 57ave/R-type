#ifndef SHOOTEMUP_COMPONENTS_SCORE_HPP
#define SHOOTEMUP_COMPONENTS_SCORE_HPP

#include <cstdint>

namespace ShootEmUp {
namespace Components {

struct Score {
    uint32_t currentScore = 0;
    uint32_t highScore = 0;
    uint32_t comboMultiplier = 1;
    float comboTimer = 0.0f;
    uint32_t consecutiveKills = 0;

    // Ajouter des points avec gestion du combo
    void AddPoints(uint32_t points) {
        currentScore += points * comboMultiplier;
        comboTimer = 3.0f;  // Reset combo timer - 3 seconds to continue combo
        consecutiveKills++;

        // Increase combo multiplier every 3 kills
        if (consecutiveKills % 3 == 0 && comboMultiplier < 5) {
            comboMultiplier++;
        }

        if (currentScore > highScore) {
            highScore = currentScore;
        }
    }

    void UpdateCombo(float deltaTime) {
        if (comboTimer > 0.0f) {
            comboTimer -= deltaTime;
            if (comboTimer <= 0.0f) {
                comboMultiplier = 1;  // Reset combo
                consecutiveKills = 0;
            }
        }
    }

    void Reset() {
        currentScore = 0;
        comboMultiplier = 1;
        comboTimer = 0.0f;
        consecutiveKills = 0;
    }
};

}  // namespace Components
}  // namespace ShootEmUp

#endif  // SHOOTEMUP_COMPONENTS_SCORE_HPP
