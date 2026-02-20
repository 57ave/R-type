/**
 * StateManager.cpp - State Manager Implementation
 */

#include "managers/StateManager.hpp"
#include "states/GameState.hpp"
#include <iostream>

StateManager::StateManager(Game* game)
    : game_(game)
{
}

StateManager::~StateManager()
{
    clearStates();
}

void StateManager::pushState(std::unique_ptr<GameState> state)
{
    if (state)
    {
        state->game_ = game_;
        
        // If there's a current state, call onExit (it will be paused)
        if (!states_.empty())
        {
            states_.back()->onExit();
        }
        
        states_.push_back(std::move(state));
        states_.back()->onEnter();
        
        std::cout << "[StateManager] Pushed state: " << states_.back()->getName() << std::endl;
    }
}

void StateManager::popState()
{
    if (!states_.empty())
    {
        std::cout << "[StateManager] Popping state: " << states_.back()->getName() << std::endl;
        states_.back()->onExit();
        states_.pop_back();
        
        // If there's a new current state, call onEnter (resume)
        if (!states_.empty())
        {
            states_.back()->onEnter();
        }
    }
}

void StateManager::changeState(std::unique_ptr<GameState> state)
{
    if (!states_.empty())
    {
        states_.back()->onExit();
        states_.pop_back();
    }
    
    if (state)
    {
        state->game_ = game_;
        states_.push_back(std::move(state));
        states_.back()->onEnter();
        
        std::cout << "[StateManager] Changed to state: " << states_.back()->getName() << std::endl;
    }
}

void StateManager::clearStates()
{
    while (!states_.empty())
    {
        states_.back()->onExit();
        states_.pop_back();
    }
    std::cout << "[StateManager] All states cleared" << std::endl;
}

void StateManager::handleEvent(const eng::engine::InputEvent& event)
{
    if (!states_.empty())
    {
        states_.back()->handleEvent(event);
    }
}

void StateManager::update(float deltaTime)
{
    if (!states_.empty())
    {
        states_.back()->update(deltaTime);
    }
}

void StateManager::render()
{
    if (!states_.empty())
    {
        states_.back()->render();
    }
}

GameState* StateManager::getCurrentState() const
{
    return states_.empty() ? nullptr : states_.back().get();
}
