#ifndef SCORE_UI_HPP
#define SCORE_UI_HPP

#include <SFML/Graphics.hpp>
#include <cstdint>
#include <string>

class ScoreUI {
public:
    ScoreUI() = default;

    void Init(const sf::Font& font, float x, float y, unsigned int fontSize = 30) {
        scoreText.setFont(font);
        scoreText.setCharacterSize(fontSize);
        scoreText.setFillColor(sf::Color::White);
        scoreText.setOutlineColor(sf::Color::Black);
        scoreText.setOutlineThickness(2.0f);
        scoreText.setPosition(x, y);

        comboText.setFont(font);
        comboText.setCharacterSize(fontSize - 8);
        comboText.setFillColor(sf::Color::Yellow);
        comboText.setOutlineColor(sf::Color::Black);
        comboText.setOutlineThickness(2.0f);
        comboText.setPosition(x, y + fontSize + 5);

        UpdateScore(0, 1, 0);
        initialized = true;
    }

    void UpdateScore(uint32_t score, uint32_t comboMultiplier = 1, uint32_t consecutiveKills = 0) {
        if (!initialized)
            return;

        scoreText.setString("Score: " + std::to_string(score));

        // Show combo if active
        if (comboMultiplier > 1) {
            comboText.setString("COMBO x" + std::to_string(comboMultiplier) + " (" +
                                std::to_string(consecutiveKills) + " kills)");
            showCombo = true;
        } else {
            showCombo = false;
        }
    }

    void Render(sf::RenderWindow& window) {
        if (!initialized)
            return;
        window.draw(scoreText);
        if (showCombo) {
            window.draw(comboText);
        }
    }

    void SetPosition(float x, float y) {
        scoreText.setPosition(x, y);
        comboText.setPosition(x, y + scoreText.getCharacterSize() + 5);
    }

    bool IsInitialized() const { return initialized; }

private:
    bool initialized = false;
    bool showCombo = false;
    sf::Text scoreText;
    sf::Text comboText;
};

#endif  // SCORE_UI_HPP
