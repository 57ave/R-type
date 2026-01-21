#ifndef HEALTHBAR_UI_HPP
#define HEALTHBAR_UI_HPP

#include <SFML/Graphics.hpp>
#include <string>

class HealthBarUI {
public:
    HealthBarUI() = default;

    void Init(float x, float y, float width, float height) {
        position = {x, y};
        size = {width, height};

        // Background (barre grise)
        background.setPosition(x, y);
        background.setSize(sf::Vector2f(width, height));
        background.setFillColor(sf::Color(50, 50, 50, 200));
        background.setOutlineColor(sf::Color::White);
        background.setOutlineThickness(2.0f);

        // Health bar (barre colorée)
        healthBar.setPosition(x + 2, y + 2);
        healthBar.setSize(sf::Vector2f(width - 4, height - 4));
        UpdateColor(100, 100);

        initialized = true;
    }

    void Update(int currentHP, int maxHP) {
        if (!initialized)
            return;

        float ratio = static_cast<float>(currentHP) / static_cast<float>(maxHP);
        if (ratio < 0.0f)
            ratio = 0.0f;
        if (ratio > 1.0f)
            ratio = 1.0f;

        float barWidth = (size.x - 4) * ratio;
        healthBar.setSize(sf::Vector2f(barWidth, size.y - 4));
        UpdateColor(currentHP, maxHP);
    }

    void Render(sf::RenderWindow& window) {
        if (!initialized)
            return;
        window.draw(background);
        window.draw(healthBar);
    }

    void SetPosition(float x, float y) {
        position = {x, y};
        background.setPosition(x, y);
        healthBar.setPosition(x + 2, y + 2);
    }

    bool IsInitialized() const { return initialized; }

private:
    void UpdateColor(int currentHP, int maxHP) {
        float ratio = static_cast<float>(currentHP) / static_cast<float>(maxHP);
        if (ratio > 0.6f) {
            healthBar.setFillColor(sf::Color(0, 255, 0, 220));  // Vert
        } else if (ratio > 0.3f) {
            healthBar.setFillColor(sf::Color(255, 255, 0, 220));  // Jaune
        } else {
            healthBar.setFillColor(sf::Color(255, 0, 0, 220));  // Rouge
        }
    }

    bool initialized = false;
    sf::Vector2f position;
    sf::Vector2f size;
    sf::RectangleShape background;
    sf::RectangleShape healthBar;
};

#endif  // HEALTHBAR_UI_HPP
