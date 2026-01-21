/**
 * PlayState.hpp - Gameplay State (Phase 6 - TODO)
 */

#pragma once

#include "GameState.hpp"

class PlayState : public GameState
{
public:
    explicit PlayState(Game* game);
    ~PlayState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const eng::engine::InputEvent& event) override;
    void update(float deltaTime) override;
    void render() override;
    const char* getName() const override { return "Play"; }
};
