/**
 * PlayState.cpp - Gameplay State Implementation (Placeholder Phase 4)
 */

#include "states/PlayState.hpp"
#include "core/Game.hpp"
#include "managers/StateManager.hpp"
#include <iostream>

PlayState::PlayState(Game* game)
{
    game_ = game;
}

void PlayState::onEnter()
{
    std::cout << "[PlayState] 🎮 Entering gameplay (PLACEHOLDER - Phase 6)" << std::endl;
    std::cout << "[PlayState] This will be implemented in Phase 6: Gameplay Core" << std::endl;
    std::cout << "[PlayState] Press ESC to return to menu" << std::endl;
}

void PlayState::onExit()
{
    std::cout << "[PlayState] Exiting gameplay" << std::endl;
}

void PlayState::handleEvent(const eng::engine::InputEvent& event)
{
    if (event.type == eng::engine::EventType::KeyPressed)
    {
        if (event.key.code == eng::engine::Key::Escape)
        {
            std::cout << "[PlayState] ESC pressed - returning to menu" << std::endl;
            game_->getStateManager()->popState();
        }
    }
}

void PlayState::update(float deltaTime)
{
    (void)deltaTime;
    // Gameplay update will be in Phase 6
}

void PlayState::render()
{
    // Draw placeholder text
    auto window = game_->getWindow();
    if (!window) return;
    
    // For now, just show a message that this is coming in Phase 6
    // The actual rendering will be implemented in Phase 6
}
