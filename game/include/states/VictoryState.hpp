#pragma once

#include "GameState.hpp"
#include <rendering/sfml/SFMLText.hpp>
#include <rendering/sfml/SFMLFont.hpp>
#include <memory>
#include <string>

class Game;

class VictoryState : public GameState
{
public:
    VictoryState(Game* game, int finalScore, int currentLevel = 3);
    ~VictoryState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const eng::engine::InputEvent& event) override;
    void update(float deltaTime) override;
    void render() override;
    const char* getName() const override { return "Victory"; }

private:
    int finalScore_;
    int currentLevel_;
    float timer_ = 0.0f;
    bool inputBlocked_ = true;

    std::unique_ptr<eng::engine::rendering::sfml::SFMLFont> font_;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> titleText_;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> scoreText_;
    std::unique_ptr<eng::engine::rendering::sfml::SFMLText> promptText_;
};
