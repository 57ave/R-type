#ifndef RTYPE_ENGINE_KEYBOARD_HPP
#define RTYPE_ENGINE_KEYBOARD_HPP

#include "engine/Input.hpp"

namespace rtype {
    namespace engine {

        // Keyboard input helper class
        class Keyboard {
        public:
            // Check if a key is currently pressed
            static bool isKeyPressed(Key key);
        };

    } // namespace engine
} // namespace rtype

#endif // RTYPE_ENGINE_KEYBOARD_HPP
