/*
** EPITECH PROJECT, 2025
** engine
** File description:
** InputManager
*/

#include "core/InputManager.hpp"

eng::core::InputManager::InputManager() : _mousePosition(0, 0) {}

eng::core::InputManager::~InputManager() {}

bool eng::core::InputManager::isKeyPressed(int keyCode) {
    auto it = _keyStates.find(keyCode);
    if (it == _keyStates.end()) {
        return false;
    }
    return it->second;
}

bool eng::core::InputManager::isKeyJustPressed(int keyCode) {
    // Retourne true seulement si :
    // - La touche est actuellement pressée
    // - ET n'était pas pressée à la frame précédente
    bool currentlyPressed = _keyStates[keyCode];
    bool previouslyPressed = _previousKeyStates[keyCode];

    return currentlyPressed && !previouslyPressed;
}

eng::engine::Vector2i eng::core::InputManager::getMousePosition() {
    return _mousePosition;
}

void eng::core::InputManager::update() {
    // Sauvegarder l'état actuel comme état précédent
    _previousKeyStates = _keyStates;

    // Note: L'état actuel sera mis à jour par le système
    // qui récupère les événements (Window ou Client)
    // Celui-ci appellera setKeyState() et setMousePosition()
}

void eng::core::InputManager::setKeyState(int keyCode, bool pressed) {
    _keyStates[keyCode] = pressed;
}

void eng::core::InputManager::setMousePosition(int x, int y) {
    _mousePosition.x = x;
    _mousePosition.y = y;
}

void eng::core::InputManager::setMousePosition(const eng::engine::Vector2i& pos) {
    _mousePosition = pos;
}