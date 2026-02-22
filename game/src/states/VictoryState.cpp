#include "states/VictoryState.hpp"
#include "core/Game.hpp"
#include "managers/StateManager.hpp"
#include "managers/NetworkManager.hpp"
#include "managers/AudioManager.hpp"
#include "states/MainMenuState.hpp"
#include <rendering/IRenderer.hpp>
#include <rendering/sfml/SFMLText.hpp>
#include <engine/Input.hpp>
#include <iostream>
#include <cmath>

VictoryState::VictoryState(Game* game, int finalScore, int currentLevel)
    : finalScore_(finalScore)
    , currentLevel_(currentLevel)
{
    game_ = game;
}

void VictoryState::onEnter()
{
    // Play victory music based on current level
    if (auto* audioManager = game_->getAudioManager()) {
        std::string musicName;
        if (currentLevel_ >= 3) {
            // All stages cleared
            musicName = "LIKE A HERO (ALL STAGE CLEAR)";
        } else {
            // Stage cleared
            musicName = "RETURN IN TRIUMPH (STAGE CLEAR)";
        }
        audioManager->playMusic(musicName, false, 1.0f);
    }
    
    int w = 1920, h = 1080;
    if (auto* win = game_->getWindow()) {
        w = win->getSize().x;
        h = win->getSize().y;
    }

    font_ = std::make_unique<eng::engine::rendering::sfml::SFMLFont>();
    if (!font_->loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "[VictoryState] Failed to load font" << std::endl;
    }

    // "VICTORY" title
    titleText_ = std::make_unique<eng::engine::rendering::sfml::SFMLText>();
    titleText_->setFont(font_.get());
    titleText_->setCharacterSize(120);
    titleText_->setFillColor(0xFFDD00FF);  // Gold
    titleText_->setString("VICTORY !");
    titleText_->setPosition(w / 2.0f - 310.0f, h / 2.0f - 180.0f);

    // Score
    scoreText_ = std::make_unique<eng::engine::rendering::sfml::SFMLText>();
    scoreText_->setFont(font_.get());
    scoreText_->setCharacterSize(50);
    scoreText_->setFillColor(0xFFFFFFFF);
    scoreText_->setString("Score final : " + std::to_string(finalScore_));
    scoreText_->setPosition(w / 2.0f - 160.0f, h / 2.0f + 20.0f);

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

void VictoryState::onExit()
{
    // Stop victory music
    if (auto* audioManager = game_->getAudioManager()) {
        audioManager->stopMusic(0.5f);
    }
    
    titleText_.reset();
    scoreText_.reset();
    promptText_.reset();
    font_.reset();
}

void VictoryState::handleEvent(const eng::engine::InputEvent& event)
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

void VictoryState::update(float deltaTime)
{
    timer_ += deltaTime;
    if (timer_ > 1.0f) inputBlocked_ = false;

    if (promptText_) {
        float alpha = inputBlocked_ ? 0.0f : (std::sin(timer_ * 3.0f) * 0.5f + 0.5f);
        uint32_t a = static_cast<uint32_t>(alpha * 255);
        promptText_->setFillColor(0xAAAAAA00 | a);
    }
}

void VictoryState::render()
{
    auto* renderer = game_->getRenderer();
    if (!renderer) return;

    if (auto* win = game_->getWindow()) {
        int w = win->getSize().x;
        int h = win->getSize().y;
        eng::engine::rendering::FloatRect overlay{0.0f, 0.0f, (float)w, (float)h};
        renderer->drawRect(overlay, 0x000022DD, 0x000022DD, 0.0f);  // Dark blue overlay
    }

    if (titleText_)  renderer->drawText(*titleText_);
    if (scoreText_)  renderer->drawText(*scoreText_);
    if (promptText_) renderer->drawText(*promptText_);
}
