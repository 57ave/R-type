/**
 * GameState.hpp - Base interface for game states
 * 
 * All game states (Menu, Play, Pause, etc.) inherit from this.
 */

#pragma once

#include <engine/Input.hpp>

// Forward declaration
class Game;

class GameState
{
public:
    virtual ~GameState() = default;

    /**
     * Called when entering this state
     */
    virtual void onEnter() = 0;

    /**
     * Called when exiting this state
     */
    virtual void onExit() = 0;

    /**
     * Handle input events
     */
    virtual void handleEvent(const eng::engine::InputEvent& event) = 0;

    /**
     * Update state logic
     * @param deltaTime Time elapsed since last update (in seconds)
     */
    virtual void update(float deltaTime) = 0;

    /**
     * Render the state
     */
    virtual void render() = 0;

    /**
     * Get the state name (for debugging)
     */
    virtual const char* getName() const = 0;

protected:
    Game* game_ = nullptr;

    friend class StateManager;
};
