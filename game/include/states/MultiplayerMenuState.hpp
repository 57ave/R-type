/**
 * MultiplayerMenuState.hpp - Multiplayer Menu State (Placeholder Phase 4)
 */

#pragma once

#include "GameState.hpp"

class MultiplayerMenuState : public GameState
{
public:
    explicit MultiplayerMenuState(Game* game);
    ~MultiplayerMenuState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const eng::engine::InputEvent& event) override;
    void update(float deltaTime) override;
    void render() override;
    const char* getName() const override { return "MultiplayerMenu"; }
};
