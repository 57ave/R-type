/*
** EPITECH PROJECT, 2025
** Input
** File description:
** Input abstraction implementation
*/

#include "engine/Input.hpp"
#include <SFML/Window/Keyboard.hpp>

namespace rtype {
    namespace engine {
        namespace internal {

            Key sfmlKeyToEngineKey(int sfmlKey) {
                // Map SFML keys to engine keys
                switch (sfmlKey) {
                    case sf::Keyboard::A: return Key::A;
                    case sf::Keyboard::B: return Key::B;
                    case sf::Keyboard::C: return Key::C;
                    case sf::Keyboard::D: return Key::D;
                    case sf::Keyboard::E: return Key::E;
                    case sf::Keyboard::F: return Key::F;
                    case sf::Keyboard::G: return Key::G;
                    case sf::Keyboard::H: return Key::H;
                    case sf::Keyboard::I: return Key::I;
                    case sf::Keyboard::J: return Key::J;
                    case sf::Keyboard::K: return Key::K;
                    case sf::Keyboard::L: return Key::L;
                    case sf::Keyboard::M: return Key::M;
                    case sf::Keyboard::N: return Key::N;
                    case sf::Keyboard::O: return Key::O;
                    case sf::Keyboard::P: return Key::P;
                    case sf::Keyboard::Q: return Key::Q;
                    case sf::Keyboard::R: return Key::R;
                    case sf::Keyboard::S: return Key::S;
                    case sf::Keyboard::T: return Key::T;
                    case sf::Keyboard::U: return Key::U;
                    case sf::Keyboard::V: return Key::V;
                    case sf::Keyboard::W: return Key::W;
                    case sf::Keyboard::X: return Key::X;
                    case sf::Keyboard::Y: return Key::Y;
                    case sf::Keyboard::Z: return Key::Z;
                    case sf::Keyboard::Num0: return Key::Num0;
                    case sf::Keyboard::Num1: return Key::Num1;
                    case sf::Keyboard::Num2: return Key::Num2;
                    case sf::Keyboard::Num3: return Key::Num3;
                    case sf::Keyboard::Num4: return Key::Num4;
                    case sf::Keyboard::Num5: return Key::Num5;
                    case sf::Keyboard::Num6: return Key::Num6;
                    case sf::Keyboard::Num7: return Key::Num7;
                    case sf::Keyboard::Num8: return Key::Num8;
                    case sf::Keyboard::Num9: return Key::Num9;
                    case sf::Keyboard::Escape: return Key::Escape;
                    case sf::Keyboard::LControl: return Key::LControl;
                    case sf::Keyboard::LShift: return Key::LShift;
                    case sf::Keyboard::LAlt: return Key::LAlt;
                    case sf::Keyboard::LSystem: return Key::LSystem;
                    case sf::Keyboard::RControl: return Key::RControl;
                    case sf::Keyboard::RShift: return Key::RShift;
                    case sf::Keyboard::RAlt: return Key::RAlt;
                    case sf::Keyboard::RSystem: return Key::RSystem;
                    case sf::Keyboard::Menu: return Key::Menu;
                    case sf::Keyboard::LBracket: return Key::LBracket;
                    case sf::Keyboard::RBracket: return Key::RBracket;
                    case sf::Keyboard::Semicolon: return Key::Semicolon;
                    case sf::Keyboard::Comma: return Key::Comma;
                    case sf::Keyboard::Period: return Key::Period;
                    case sf::Keyboard::Quote: return Key::Quote;
                    case sf::Keyboard::Slash: return Key::Slash;
                    case sf::Keyboard::Backslash: return Key::Backslash;
                    case sf::Keyboard::Tilde: return Key::Tilde;
                    case sf::Keyboard::Equal: return Key::Equal;
                    case sf::Keyboard::Hyphen: return Key::Hyphen;
                    case sf::Keyboard::Space: return Key::Space;
                    case sf::Keyboard::Enter: return Key::Enter;
                    case sf::Keyboard::Backspace: return Key::Backspace;
                    case sf::Keyboard::Tab: return Key::Tab;
                    case sf::Keyboard::PageUp: return Key::PageUp;
                    case sf::Keyboard::PageDown: return Key::PageDown;
                    case sf::Keyboard::End: return Key::End;
                    case sf::Keyboard::Home: return Key::Home;
                    case sf::Keyboard::Insert: return Key::Insert;
                    case sf::Keyboard::Delete: return Key::Delete;
                    case sf::Keyboard::Add: return Key::Add;
                    case sf::Keyboard::Subtract: return Key::Subtract;
                    case sf::Keyboard::Multiply: return Key::Multiply;
                    case sf::Keyboard::Divide: return Key::Divide;
                    case sf::Keyboard::Left: return Key::Left;
                    case sf::Keyboard::Right: return Key::Right;
                    case sf::Keyboard::Up: return Key::Up;
                    case sf::Keyboard::Down: return Key::Down;
                    case sf::Keyboard::Numpad0: return Key::Numpad0;
                    case sf::Keyboard::Numpad1: return Key::Numpad1;
                    case sf::Keyboard::Numpad2: return Key::Numpad2;
                    case sf::Keyboard::Numpad3: return Key::Numpad3;
                    case sf::Keyboard::Numpad4: return Key::Numpad4;
                    case sf::Keyboard::Numpad5: return Key::Numpad5;
                    case sf::Keyboard::Numpad6: return Key::Numpad6;
                    case sf::Keyboard::Numpad7: return Key::Numpad7;
                    case sf::Keyboard::Numpad8: return Key::Numpad8;
                    case sf::Keyboard::Numpad9: return Key::Numpad9;
                    case sf::Keyboard::F1: return Key::F1;
                    case sf::Keyboard::F2: return Key::F2;
                    case sf::Keyboard::F3: return Key::F3;
                    case sf::Keyboard::F4: return Key::F4;
                    case sf::Keyboard::F5: return Key::F5;
                    case sf::Keyboard::F6: return Key::F6;
                    case sf::Keyboard::F7: return Key::F7;
                    case sf::Keyboard::F8: return Key::F8;
                    case sf::Keyboard::F9: return Key::F9;
                    case sf::Keyboard::F10: return Key::F10;
                    case sf::Keyboard::F11: return Key::F11;
                    case sf::Keyboard::F12: return Key::F12;
                    case sf::Keyboard::F13: return Key::F13;
                    case sf::Keyboard::F14: return Key::F14;
                    case sf::Keyboard::F15: return Key::F15;
                    case sf::Keyboard::Pause: return Key::Pause;
                    default: return Key::Unknown;
                }
            }

            int engineKeyToSfmlKey(Key key) {
                // Map engine keys to SFML keys
                switch (key) {
                    case Key::A: return sf::Keyboard::A;
                    case Key::B: return sf::Keyboard::B;
                    case Key::C: return sf::Keyboard::C;
                    case Key::D: return sf::Keyboard::D;
                    case Key::E: return sf::Keyboard::E;
                    case Key::F: return sf::Keyboard::F;
                    case Key::G: return sf::Keyboard::G;
                    case Key::H: return sf::Keyboard::H;
                    case Key::I: return sf::Keyboard::I;
                    case Key::J: return sf::Keyboard::J;
                    case Key::K: return sf::Keyboard::K;
                    case Key::L: return sf::Keyboard::L;
                    case Key::M: return sf::Keyboard::M;
                    case Key::N: return sf::Keyboard::N;
                    case Key::O: return sf::Keyboard::O;
                    case Key::P: return sf::Keyboard::P;
                    case Key::Q: return sf::Keyboard::Q;
                    case Key::R: return sf::Keyboard::R;
                    case Key::S: return sf::Keyboard::S;
                    case Key::T: return sf::Keyboard::T;
                    case Key::U: return sf::Keyboard::U;
                    case Key::V: return sf::Keyboard::V;
                    case Key::W: return sf::Keyboard::W;
                    case Key::X: return sf::Keyboard::X;
                    case Key::Y: return sf::Keyboard::Y;
                    case Key::Z: return sf::Keyboard::Z;
                    case Key::Num0: return sf::Keyboard::Num0;
                    case Key::Num1: return sf::Keyboard::Num1;
                    case Key::Num2: return sf::Keyboard::Num2;
                    case Key::Num3: return sf::Keyboard::Num3;
                    case Key::Num4: return sf::Keyboard::Num4;
                    case Key::Num5: return sf::Keyboard::Num5;
                    case Key::Num6: return sf::Keyboard::Num6;
                    case Key::Num7: return sf::Keyboard::Num7;
                    case Key::Num8: return sf::Keyboard::Num8;
                    case Key::Num9: return sf::Keyboard::Num9;
                    case Key::Escape: return sf::Keyboard::Escape;
                    case Key::LControl: return sf::Keyboard::LControl;
                    case Key::LShift: return sf::Keyboard::LShift;
                    case Key::LAlt: return sf::Keyboard::LAlt;
                    case Key::LSystem: return sf::Keyboard::LSystem;
                    case Key::RControl: return sf::Keyboard::RControl;
                    case Key::RShift: return sf::Keyboard::RShift;
                    case Key::RAlt: return sf::Keyboard::RAlt;
                    case Key::RSystem: return sf::Keyboard::RSystem;
                    case Key::Menu: return sf::Keyboard::Menu;
                    case Key::LBracket: return sf::Keyboard::LBracket;
                    case Key::RBracket: return sf::Keyboard::RBracket;
                    case Key::Semicolon: return sf::Keyboard::Semicolon;
                    case Key::Comma: return sf::Keyboard::Comma;
                    case Key::Period: return sf::Keyboard::Period;
                    case Key::Quote: return sf::Keyboard::Quote;
                    case Key::Slash: return sf::Keyboard::Slash;
                    case Key::Backslash: return sf::Keyboard::Backslash;
                    case Key::Tilde: return sf::Keyboard::Tilde;
                    case Key::Equal: return sf::Keyboard::Equal;
                    case Key::Hyphen: return sf::Keyboard::Hyphen;
                    case Key::Space: return sf::Keyboard::Space;
                    case Key::Enter: return sf::Keyboard::Enter;
                    case Key::Backspace: return sf::Keyboard::Backspace;
                    case Key::Tab: return sf::Keyboard::Tab;
                    case Key::PageUp: return sf::Keyboard::PageUp;
                    case Key::PageDown: return sf::Keyboard::PageDown;
                    case Key::End: return sf::Keyboard::End;
                    case Key::Home: return sf::Keyboard::Home;
                    case Key::Insert: return sf::Keyboard::Insert;
                    case Key::Delete: return sf::Keyboard::Delete;
                    case Key::Add: return sf::Keyboard::Add;
                    case Key::Subtract: return sf::Keyboard::Subtract;
                    case Key::Multiply: return sf::Keyboard::Multiply;
                    case Key::Divide: return sf::Keyboard::Divide;
                    case Key::Left: return sf::Keyboard::Left;
                    case Key::Right: return sf::Keyboard::Right;
                    case Key::Up: return sf::Keyboard::Up;
                    case Key::Down: return sf::Keyboard::Down;
                    case Key::Numpad0: return sf::Keyboard::Numpad0;
                    case Key::Numpad1: return sf::Keyboard::Numpad1;
                    case Key::Numpad2: return sf::Keyboard::Numpad2;
                    case Key::Numpad3: return sf::Keyboard::Numpad3;
                    case Key::Numpad4: return sf::Keyboard::Numpad4;
                    case Key::Numpad5: return sf::Keyboard::Numpad5;
                    case Key::Numpad6: return sf::Keyboard::Numpad6;
                    case Key::Numpad7: return sf::Keyboard::Numpad7;
                    case Key::Numpad8: return sf::Keyboard::Numpad8;
                    case Key::Numpad9: return sf::Keyboard::Numpad9;
                    case Key::F1: return sf::Keyboard::F1;
                    case Key::F2: return sf::Keyboard::F2;
                    case Key::F3: return sf::Keyboard::F3;
                    case Key::F4: return sf::Keyboard::F4;
                    case Key::F5: return sf::Keyboard::F5;
                    case Key::F6: return sf::Keyboard::F6;
                    case Key::F7: return sf::Keyboard::F7;
                    case Key::F8: return sf::Keyboard::F8;
                    case Key::F9: return sf::Keyboard::F9;
                    case Key::F10: return sf::Keyboard::F10;
                    case Key::F11: return sf::Keyboard::F11;
                    case Key::F12: return sf::Keyboard::F12;
                    case Key::F13: return sf::Keyboard::F13;
                    case Key::F14: return sf::Keyboard::F14;
                    case Key::F15: return sf::Keyboard::F15;
                    case Key::Pause: return sf::Keyboard::Pause;
                    default: return -1;
                }
            }

        } // namespace internal
    } // namespace engine
} // namespace rtype
