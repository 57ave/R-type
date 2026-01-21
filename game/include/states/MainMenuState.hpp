/**
 * MainMenuState.hpp - Main Menu State
 * 
 * The initial state when the game starts.
 */

#pragma once

#include "GameState.hpp"
#include <ecs/Coordinator.hpp>
#include <vector>

class MainMenuState : public GameState
{
public:
    explicit MainMenuState(Game* game);
    ~MainMenuState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const eng::engine::InputEvent& event) override;
    void update(float deltaTime) override;
    void render() override;
    const char* getName() const override { return "MainMenu"; }

private:
    std::vector<ECS::Entity> menuEntities_;  // UI entities created for this menu
    ECS::Entity hoveredButton_ = 0;          // Currently hovered button entity
};
