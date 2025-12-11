/*
** EPITECH PROJECT, 2025
** InputManager
** File description:
** rtype
*/

#ifndef _INPUTE_MANAGER_CORE_
    #define _INPUTE_MANAGER_CORE_
    #include "Types.hpp"
    #include <unordered_map>
    #include <string>

    namespace rtype {
        namespace core {

            class InputManager {
                public:
                    InputManager();
                    ~InputManager();
                    
                    // Query input states
                    bool isKeyPressed(int keyCode);
                    bool isKeyJustPressed(int keyCode);
                    engine::Vector2i getMousePosition();
                    
                    // Update internal state (called each frame)
                    void update();

                    // Set input states (called by Window/Client)
                    void setKeyState(int keyCode, bool pressed);
                    void setMousePosition(int x, int y);
                    void setMousePosition(const engine::Vector2i& pos);

                protected:
                private:
                    engine::Vector2i _mousePosition;
                    std::unordered_map<int, bool> _keyStates;
                    std::unordered_map<int, bool> _previousKeyStates;
            };

        } // namespace core
    } // namespace rtype

#endif /* !_INPUTE_MANAGER_CORE_ */
