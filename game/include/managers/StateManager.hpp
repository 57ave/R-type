/**
 * StateManager.hpp - Manages game states stack
 * 
 * Handles transitions between different game states.
 */

#pragma once

#include <memory>
#include <vector>
#include <engine/Input.hpp>

class GameState;
class Game;

class StateManager
{
public:
    explicit StateManager(Game* game);
    ~StateManager();

    /**
     * Push a new state onto the stack
     */
    void pushState(std::unique_ptr<GameState> state);

    /**
     * Pop the current state
     */
    void popState();

    /**
     * Replace the current state with a new one
     */
    void changeState(std::unique_ptr<GameState> state);

    /**
     * Clear all states
     */
    void clearStates();

    /**
     * Handle input events for current state
     */
    void handleEvent(const eng::engine::InputEvent& event);

    /**
     * Update current state
     */
    void update(float deltaTime);

    /**
     * Render current state
     */
    void render();

    /**
     * Check if there are any states
     */
    bool hasStates() const { return !states_.empty(); }

    /**
     * Get current state
     */
    GameState* getCurrentState() const;

private:
    Game* game_;
    std::vector<std::unique_ptr<GameState>> states_;
    
    // Pending operations (to avoid modifying stack during iteration)
    enum class Operation { None, Push, Pop, Change, Clear };
    Operation pendingOp_ = Operation::None;
    std::unique_ptr<GameState> pendingState_;
};
