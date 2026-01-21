#ifndef ENG_ENGINE_KEYBOARD_HPP
#define ENG_ENGINE_KEYBOARD_HPP

#include "engine/Input.hpp"

namespace eng {
namespace engine {

// Keyboard input helper class
class Keyboard {
public:
    // Check if a key is currently pressed
    static bool isKeyPressed(Key key);
};

}  // namespace engine
}  // namespace eng

#endif  // ENG_ENGINE_KEYBOARD_HPP
