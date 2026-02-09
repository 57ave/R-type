/**
 * PlayState.cpp - Gameplay State Implementation (Phase 6 - TODO)
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
    std::cout << "[PlayState] This will be implemented after Phase 5 is complete" << std::endl;
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
    // Gameplay rendering will be in Phase 6
}
