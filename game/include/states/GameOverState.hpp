#pragma once

#include "GameState.hpp"
#include <rendering/sfml/SFMLText.hpp>
#include <rendering/sfml/SFMLFont.hpp>
#include <memory>
#include <string>

class Game;

class GameOverState : public GameState
{
public:
    GameOverState(Game* game, int finalScore);
    ~GameOverState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const eng::engine::InputEvent& event) override;
    void update(float deltaTime) override;
    void render() override;
    const char* getName() const override { return "GameOver"; }

private:
    int finalScore_;
    float timer_ = 0.0f;          // Auto-return to menu after delay
    bool inputBlocked_ = true;     // Block input briefly to avoid accidental skip

    std::unique_ptr<eng::engine::rendering::sfml::SFMLFont> font_;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> titleText_;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> scoreText_;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> promptText_;
};
