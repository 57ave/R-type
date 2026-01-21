/**
 * MultiplayerMenuState.cpp - Multiplayer Menu Implementation (Placeholder Phase 4)
 */

#include "states/MultiplayerMenuState.hpp"
#include "core/Game.hpp"
#include "managers/StateManager.hpp"
#include <iostream>

MultiplayerMenuState::MultiplayerMenuState(Game* game)
{
    game_ = game;
}

void MultiplayerMenuState::onEnter()
{
    std::cout << "[MultiplayerMenuState] 🌐 Entering multiplayer menu (PLACEHOLDER - Phase 5)" << std::endl;
    std::cout << "[MultiplayerMenuState] This will be implemented in Phase 5: Network & Multiplayer" << std::endl;
    std::cout << "[MultiplayerMenuState] Press ESC to return to menu" << std::endl;
}

void MultiplayerMenuState::onExit()
{
    std::cout << "[MultiplayerMenuState] Exiting multiplayer menu" << std::endl;
}

void MultiplayerMenuState::handleEvent(const eng::engine::InputEvent& event)
{
    if (event.type == eng::engine::EventType::KeyPressed)
    {
        if (event.key.code == eng::engine::Key::Escape)
        {
            std::cout << "[MultiplayerMenuState] ESC pressed - returning to menu" << std::endl;
            game_->getStateManager()->popState();
        }
    }
}

void MultiplayerMenuState::update(float deltaTime)
{
    (void)deltaTime;
    // Will be implemented in Phase 5
}

void MultiplayerMenuState::render()
{
    // Placeholder rendering
    // Will be implemented in Phase 5 with lobby, player list, etc.
}
