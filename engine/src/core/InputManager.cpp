/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** InputManager
*/

#include "core/InputManager.hpp"

rtype::core::InputManager::InputManager() 
    : _mousePosition(0, 0) {
}

rtype::core::InputManager::~InputManager() {
}

bool rtype::core::InputManager::isKeyPressed(int keyCode) {
    auto it = _keyStates.find(keyCode);
    if (it == _keyStates.end()) {
        return false;
    }
    return it->second;
}

bool rtype::core::InputManager::isKeyJustPressed(int keyCode) {
    // Retourne true seulement si :
    // - La touche est actuellement pressée
    // - ET n'était pas pressée à la frame précédente
    bool currentlyPressed = _keyStates[keyCode];
    bool previouslyPressed = _previousKeyStates[keyCode];
    
    return currentlyPressed && !previouslyPressed;
}

rtype::engine::Vector2i rtype::core::InputManager::getMousePosition() {
    return _mousePosition;
}

void rtype::core::InputManager::update() {
    // Sauvegarder l'état actuel comme état précédent
    _previousKeyStates = _keyStates;
    
    // Note: L'état actuel sera mis à jour par le système 
    // qui récupère les événements (Window ou Client)
    // Celui-ci appellera setKeyState() et setMousePosition()
}

void rtype::core::InputManager::setKeyState(int keyCode, bool pressed) {
    _keyStates[keyCode] = pressed;
}

void rtype::core::InputManager::setMousePosition(int x, int y) {
    _mousePosition.x = x;
    _mousePosition.y = y;
}

void rtype::core::InputManager::setMousePosition(const rtype::engine::Vector2i& pos) {
    _mousePosition = pos;
}