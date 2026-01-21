#ifndef ENG_ENGINE_INPUT_HPP
#define ENG_ENGINE_INPUT_HPP

#include <cstdint>

namespace eng {
namespace engine {

// Key codes (mapped to common keyboard keys)
enum class Key {
    Unknown = -1,
    A = 0,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,
    Num0,
    Num1,
    Num2,
    Num3,
    Num4,
    Num5,
    Num6,
    Num7,
    Num8,
    Num9,
    Escape,
    LControl,
    LShift,
    LAlt,
    LSystem,
    RControl,
    RShift,
    RAlt,
    RSystem,
    Menu,
    LBracket,
    RBracket,
    Semicolon,
    Comma,
    Period,
    Quote,
    Slash,
    Backslash,
    Tilde,
    Equal,
    Hyphen,
    Space,
    Enter,
    Backspace,
    Tab,
    PageUp,
    PageDown,
    End,
    Home,
    Insert,
    Delete,
    Add,
    Subtract,
    Multiply,
    Divide,
    Left,
    Right,
    Up,
    Down,
    Numpad0,
    Numpad1,
    Numpad2,
    Numpad3,
    Numpad4,
    Numpad5,
    Numpad6,
    Numpad7,
    Numpad8,
    Numpad9,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    Pause,
    KeyCount
};

// Event types
enum class EventType {
    Closed,
    Resized,
    LostFocus,
    GainedFocus,
    KeyPressed,
    KeyReleased,
    MouseMoved,
    MouseButtonPressed,
    MouseButtonReleased,
    MouseWheelScrolled,
    TextEntered  // Ajouté pour la saisie de texte
};

// Input event structure
struct InputEvent {
    EventType type;

    // Key event data
    struct KeyEvent {
        Key code;
        bool alt;
        bool control;
        bool shift;
        bool system;
    } key;

    // Mouse move event data
    struct MouseMoveEvent {
        int x;
        int y;
    } mouseMove;

    // Mouse button event data
    struct MouseButtonEvent {
        int button;  // 0 = left, 1 = right, 2 = middle
        int x;
        int y;
    } mouseButton;

    // Mouse wheel event data
    struct MouseWheelScrollEvent {
        float delta;
        int x;
        int y;
    } mouseWheelScroll;

    // Size event data
    struct SizeEvent {
        unsigned int width;
        unsigned int height;
    } size;

    // Text input event data
    struct TextEvent {
        unsigned int unicode;  // UTF-32 codepoint
    } text;
};

// Key mapping from SFML to engine (internal use)
namespace internal {
Key sfmlKeyToEngineKey(int sfmlKey);
int engineKeyToSfmlKey(Key key);
}  // namespace internal

}  // namespace engine
}  // namespace eng

#endif  // ENG_ENGINE_INPUT_HPP
