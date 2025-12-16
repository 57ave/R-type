/*
** EPITECH PROJECT, 2025
** Keyboard
** File description:
** Keyboard input helper implementation
*/

#include "engine/Keyboard.hpp"
#include <SFML/Window/Keyboard.hpp>

namespace rtype {
    namespace engine {

        bool Keyboard::isKeyPressed(Key key) {
            int sfmlKey = internal::engineKeyToSfmlKey(key);
            if (sfmlKey == -1) {
                return false;
            }
            return sf::Keyboard::isKeyPressed(static_cast<sf::Keyboard::Key>(sfmlKey));
        }

    } // namespace engine
} // namespace rtype
