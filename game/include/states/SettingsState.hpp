/**
 * SettingsState.hpp - Settings Menu State
 */

#pragma once

#include "GameState.hpp"
#include <ecs/Coordinator.hpp>
#include <vector>

class SettingsState : public GameState
{
public:
    explicit SettingsState(Game* game);
    ~SettingsState() override = default;

    void onEnter() override;
    void onExit() override;
    void handleEvent(const eng::engine::InputEvent& event) override;
    void update(float deltaTime) override;
    void render() override;
    const char* getName() const override { return "Settings"; }

private:
    std::vector<ECS::Entity> menuEntities_;
    ECS::Entity hoveredButton_ = 0;
    
    // Settings entities (for saving)
    ECS::Entity masterVolumeSlider_ = 0;
    ECS::Entity musicVolumeSlider_ = 0;
    ECS::Entity sfxVolumeSlider_ = 0;
    ECS::Entity vsyncCheckbox_ = 0;
    ECS::Entity fullscreenCheckbox_ = 0;
    ECS::Entity showFpsCheckbox_ = 0;
    
    void saveSettings();
    void loadSettings();
};
