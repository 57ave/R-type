#include "core/InputHandler.hpp"
#include <engine/Keyboard.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <math.h>

namespace RType::Core {

InputHandler::InputHandler()
    : debugMode(false)
{
    // Configuration par défaut des touches - ZQSD (FR) + Flèches
    MapKey(InputAction::MoveUp, eng::engine::Key::Z);
    MapKey(InputAction::MoveDown, eng::engine::Key::S);
    MapKey(InputAction::MoveLeft, eng::engine::Key::Q);
    MapKey(InputAction::MoveRight, eng::engine::Key::D);
    
    // Touches flèches alternatives
    MapKey(InputAction::MoveUp, eng::engine::Key::Up);
    MapKey(InputAction::MoveDown, eng::engine::Key::Down);
    MapKey(InputAction::MoveLeft, eng::engine::Key::Left);
    MapKey(InputAction::MoveRight, eng::engine::Key::Right);
    
    MapKey(InputAction::Shoot, eng::engine::Key::Space);
    MapKey(InputAction::Pause, eng::engine::Key::Escape);
    MapKey(InputAction::Confirm, eng::engine::Key::Enter);
    MapKey(InputAction::Cancel, eng::engine::Key::Escape);
    MapKey(InputAction::Console, eng::engine::Key::F1);
    MapKey(InputAction::Menu, eng::engine::Key::M);
    
    std::cout << "[InputHandler] Initialized with default key mapping (ZQSD + Arrows)" << std::endl;
}

InputHandler::~InputHandler() {
    std::cout << "[InputHandler] Destroyed" << std::endl;
}

void InputHandler::MapKey(InputAction action, eng::engine::Key keyCode) {
    keyToAction[keyCode] = action;
    
    // Initialiser l'état de l'action si nécessaire
    if (actionStates.find(action) == actionStates.end()) {
        actionStates[action] = ActionState{};
    }
    
    if (debugMode) {
        std::cout << "[InputHandler] Mapped " << KeyToString(keyCode) 
                  << " to " << ActionToString(action) << std::endl;
    }
}

bool InputHandler::LoadKeyMapping(const std::string& configPath) {
    // TODO: Implémenter le chargement depuis Lua
    std::cout << "[InputHandler] Loading key mapping from: " << configPath << std::endl;
    // Pour l'instant, on garde la configuration par défaut
    return true;
}

void InputHandler::SaveKeyMapping(const std::string& configPath) {
    // TODO: Implémenter la sauvegarde vers Lua
    std::cout << "[InputHandler] Saving key mapping to: " << configPath << std::endl;
}

void InputHandler::Update(float deltaTime) {
    // Reset des flags "just pressed/released"
    for (auto& pair : actionStates) {
        pair.second.justPressed = false;
        pair.second.justReleased = false;
        
        // Mise à jour du temps de maintien
        if (pair.second.pressed) {
            pair.second.holdTime += deltaTime;
        }
    }
    
    // Vérifier directement les touches pour chaque action (supporte plusieurs touches)
    // MoveUp: Z ou Flèche Haut
    bool moveUpPressed = eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Z) ||
                         eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Up);
    UpdateActionState(InputAction::MoveUp, moveUpPressed, deltaTime);
    
    // MoveDown: S ou Flèche Bas
    bool moveDownPressed = eng::engine::Keyboard::isKeyPressed(eng::engine::Key::S) ||
                           eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Down);
    UpdateActionState(InputAction::MoveDown, moveDownPressed, deltaTime);
    
    // MoveLeft: Q ou Flèche Gauche
    bool moveLeftPressed = eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Q) ||
                           eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Left);
    UpdateActionState(InputAction::MoveLeft, moveLeftPressed, deltaTime);
    
    // MoveRight: D ou Flèche Droite
    bool moveRightPressed = eng::engine::Keyboard::isKeyPressed(eng::engine::Key::D) ||
                            eng::engine::Keyboard::isKeyPressed(eng::engine::Key::Right);
    UpdateActionState(InputAction::MoveRight, moveRightPressed, deltaTime);
    
    // Autres actions via le mapping standard
    for (const auto& pair : keyToAction) {
        eng::engine::Key key = pair.first;
        InputAction action = pair.second;
        
        // Skip movement actions (already handled above)
        if (action == InputAction::MoveUp || action == InputAction::MoveDown ||
            action == InputAction::MoveLeft || action == InputAction::MoveRight) {
            continue;
        }
        
        bool currentlyPressed = eng::engine::Keyboard::isKeyPressed(key);
        UpdateActionState(action, currentlyPressed, deltaTime);
    }
}

void InputHandler::HandleEvent(const eng::engine::InputEvent& event) {
    if (event.type == eng::engine::EventType::KeyPressed) {
        auto it = keyToAction.find(event.key.code);
        if (it != keyToAction.end()) {
            InputAction action = it->second;
            UpdateActionState(action, true, 0.0f);
            
            if (debugMode) {
                std::cout << "[InputHandler] Key pressed: " << KeyToString(event.key.code) 
                          << " -> " << ActionToString(action) << std::endl;
            }
        }
    }
    else if (event.type == eng::engine::EventType::KeyReleased) {
        auto it = keyToAction.find(event.key.code);
        if (it != keyToAction.end()) {
            InputAction action = it->second;
            UpdateActionState(action, false, 0.0f);
            
            if (debugMode) {
                std::cout << "[InputHandler] Key released: " << KeyToString(event.key.code) 
                          << " -> " << ActionToString(action) << std::endl;
            }
        }
    }
}

void InputHandler::UpdateActionState(InputAction action, bool pressed, float deltaTime) {
    auto& state = actionStates[action];
    bool wasPressed = state.pressed;
    
    state.pressed = pressed;
    
    if (pressed && !wasPressed) {
        // Action vient d'être pressée
        state.justPressed = true;
        state.holdTime = 0.0f;
        TriggerActionCallback(action);
    }
    else if (!pressed && wasPressed) {
        // Action vient d'être relâchée
        state.justReleased = true;
        state.holdTime = 0.0f;
    }
}

bool InputHandler::IsActionPressed(InputAction action) const {
    auto it = actionStates.find(action);
    return it != actionStates.end() && it->second.pressed;
}

bool InputHandler::IsActionJustPressed(InputAction action) const {
    auto it = actionStates.find(action);
    return it != actionStates.end() && it->second.justPressed;
}

bool InputHandler::IsActionJustReleased(InputAction action) const {
    auto it = actionStates.find(action);
    return it != actionStates.end() && it->second.justReleased;
}

float InputHandler::GetActionHoldTime(InputAction action) const {
    auto it = actionStates.find(action);
    return it != actionStates.end() ? it->second.holdTime : 0.0f;
}

void InputHandler::SetActionCallback(InputAction action, std::function<void()> callback) {
    actionCallbacks[action] = callback;
    std::cout << "[InputHandler] Set callback for action: " << ActionToString(action) << std::endl;
}

void InputHandler::RemoveActionCallback(InputAction action) {
    actionCallbacks.erase(action);
    std::cout << "[InputHandler] Removed callback for action: " << ActionToString(action) << std::endl;
}

void InputHandler::TriggerActionCallback(InputAction action) {
    auto it = actionCallbacks.find(action);
    if (it != actionCallbacks.end() && it->second) {
        it->second();
    }
}

uint8_t InputHandler::GetNetworkInputMask() const {
    uint8_t mask = 0;
    
    if (IsActionPressed(InputAction::MoveUp))    mask |= (1 << 0);
    if (IsActionPressed(InputAction::MoveDown))  mask |= (1 << 1);
    if (IsActionPressed(InputAction::MoveLeft))  mask |= (1 << 2);
    if (IsActionPressed(InputAction::MoveRight)) mask |= (1 << 3);
    if (IsActionPressed(InputAction::Shoot))     mask |= (1 << 4);
    
    return mask;
}

void InputHandler::ApplyNetworkInputMask(uint8_t mask) {
    // Appliquer le masque reçu du réseau aux actions
    // Utilisé pour les joueurs distants
    UpdateActionState(InputAction::MoveUp,    (mask & (1 << 0)) != 0, 0.0f);
    UpdateActionState(InputAction::MoveDown,  (mask & (1 << 1)) != 0, 0.0f);
    UpdateActionState(InputAction::MoveLeft,  (mask & (1 << 2)) != 0, 0.0f);
    UpdateActionState(InputAction::MoveRight, (mask & (1 << 3)) != 0, 0.0f);
    UpdateActionState(InputAction::Shoot,     (mask & (1 << 4)) != 0, 0.0f);
}

std::pair<float, float> InputHandler::GetMovementVector() const {
    float x = 0.0f, y = 0.0f;
    
    if (IsActionPressed(InputAction::MoveLeft))  x -= 1.0f;
    if (IsActionPressed(InputAction::MoveRight)) x += 1.0f;
    if (IsActionPressed(InputAction::MoveUp))    y -= 1.0f;
    if (IsActionPressed(InputAction::MoveDown))  y += 1.0f;
    
    // Normaliser le vecteur diagonal
    if (x != 0.0f && y != 0.0f) {
        float length = std::sqrt(x * x + y * y);
        x /= length;
        y /= length;
    }
    
    return {x, y};
}

void InputHandler::SetDebugMode(bool enabled) {
    debugMode = enabled;
    std::cout << "[InputHandler] Debug mode: " << (enabled ? "enabled" : "disabled") << std::endl;
}

void InputHandler::DebugPrintState() const {
    std::cout << "[InputHandler] Current state:" << std::endl;
    for (const auto& pair : actionStates) {
        const auto& state = pair.second;
        std::cout << "  " << ActionToString(pair.first) 
                  << ": pressed=" << (state.pressed ? "true" : "false")
                  << ", justPressed=" << (state.justPressed ? "true" : "false")
                  << ", justReleased=" << (state.justReleased ? "true" : "false")
                  << ", holdTime=" << state.holdTime << std::endl;
    }
}

std::string InputHandler::ActionToString(InputAction action) const {
    switch (action) {
        case InputAction::MoveUp:    return "MoveUp";
        case InputAction::MoveDown:  return "MoveDown";
        case InputAction::MoveLeft:  return "MoveLeft";
        case InputAction::MoveRight: return "MoveRight";
        case InputAction::Shoot:     return "Shoot";
        case InputAction::Pause:     return "Pause";
        case InputAction::Confirm:   return "Confirm";
        case InputAction::Cancel:    return "Cancel";
        case InputAction::Console:   return "Console";
        case InputAction::Menu:      return "Menu";
        default:                     return "Unknown";
    }
}

std::string InputHandler::KeyToString(eng::engine::Key key) const {
    switch (key) {
        case eng::engine::Key::Z: return "Z";
        case eng::engine::Key::S: return "S";
        case eng::engine::Key::Q: return "Q";
        case eng::engine::Key::D: return "D";
        case eng::engine::Key::Space: return "Space";
        case eng::engine::Key::Escape: return "Escape";
        case eng::engine::Key::Enter: return "Enter";
        case eng::engine::Key::F1: return "F1";
        case eng::engine::Key::M: return "M";
        default: return "Key" + std::to_string(static_cast<int>(key));
    }
}

} // namespace RType::Core
