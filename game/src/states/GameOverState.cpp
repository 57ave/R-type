#include "states/GameOverState.hpp"
#include "core/Game.hpp"
#include "managers/StateManager.hpp"
#include "managers/NetworkManager.hpp"
#include "states/MainMenuState.hpp"
#include <rendering/IRenderer.hpp>
#include <rendering/sfml/SFMLText.hpp>
#include <engine/Input.hpp>
#include <iostream>

GameOverState::GameOverState(Game* game, int finalScore)
    : finalScore_(finalScore)
{
    game_ = game;
}

void GameOverState::onEnter()
{
    int w = 1920, h = 1080;
    if (auto* win = game_->getWindow()) {
        w = win->getSize().x;
        h = win->getSize().y;
    }

    font_ = std::make_unique<eng::engine::rendering::sfml::SFMLFont>();
    if (!font_->loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "[GameOverState] Failed to load font" << std::endl;
    }

    // "GAME OVER" title
    titleText_ = std::make_unique<eng::engine::rendering::sfml::SFMLText>();
    titleText_->setFont(font_.get());
    titleText_->setCharacterSize(120);
    titleText_->setFillColor(0xFF2222FF);  // Red
    titleText_->setString("GAME OVER");
    titleText_->setPosition(w / 2.0f - 370.0f, h / 2.0f - 180.0f);

    // Score
    scoreText_ = std::make_unique<eng::engine::rendering::sfml::SFMLText>();
    scoreText_->setFont(font_.get());
    scoreText_->setCharacterSize(50);
    scoreText_->setFillColor(0xFFFFFFFF);
    scoreText_->setString("Score : " + std::to_string(finalScore_));
    scoreText_->setPosition(w / 2.0f - 130.0f, h / 2.0f + 20.0f);

    // Prompt
    promptText_ = std::make_unique<eng::engine::rendering::sfml::SFMLText>();
    promptText_->setFont(font_.get());
    promptText_->setCharacterSize(32);
    promptText_->setFillColor(0xAAAAAAFF);
    promptText_->setString("Appuie sur Entree pour quitter");
    promptText_->setPosition(w / 2.0f - 310.0f, h / 2.0f + 110.0f);

    timer_ = 0.0f;
    inputBlocked_ = true;
}

void GameOverState::onExit()
{
    titleText_.reset();
    scoreText_.reset();
    promptText_.reset();
    font_.reset();
}

void GameOverState::handleEvent(const eng::engine::InputEvent& event)
{
    if (inputBlocked_) return;

    if (event.type == eng::engine::EventType::KeyPressed) {
        if (event.key.code == eng::engine::Key::Enter ||
            event.key.code == eng::engine::Key::Space ||
            event.key.code == eng::engine::Key::Escape) {
            // Disconnect from server before returning to menu
            auto* networkManager = game_->getNetworkManager();
            if (networkManager && networkManager->isConnected()) {
                networkManager->leaveRoom();
            }
            game_->getStateManager()->changeState(std::make_unique<MainMenuState>(game_));
        }
    }
}

void GameOverState::update(float deltaTime)
{
    timer_ += deltaTime;
    if (timer_ > 1.0f) inputBlocked_ = false;

    // Blink prompt text after input is unblocked
    if (promptText_) {
        float alpha = inputBlocked_ ? 0.0f : (std::sin(timer_ * 3.0f) * 0.5f + 0.5f);
        uint32_t a = static_cast<uint32_t>(alpha * 255);
        promptText_->setFillColor(0xAAAAAA00 | a);
    }
}

void GameOverState::render()
{
    auto* renderer = game_->getRenderer();
    if (!renderer) return;

    // Dark overlay
    if (auto* win = game_->getWindow()) {
        int w = win->getSize().x;
        int h = win->getSize().y;
        eng::engine::rendering::FloatRect overlay{0.0f, 0.0f, (float)w, (float)h};
        renderer->drawRect(overlay, 0x000000CC, 0x000000CC, 0.0f);
    }

    if (titleText_)  renderer->drawText(*titleText_);
    if (scoreText_)  renderer->drawText(*scoreText_);
    if (promptText_) renderer->drawText(*promptText_);
}
