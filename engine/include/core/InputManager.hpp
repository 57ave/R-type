/*
** EPITECH PROJECT, 2025
** InputManager
** File description:
** rtype
*/

#ifndef _INPUTE_MANAGER_CORE_
    #define _INPUTE_MANAGER_CORE_
    #include <unordered_map>
    class InputManager {
        public:
            InputManager();
            ~InputManager();
            bool isKeyPressed(int keyCode);
            bool isKeyJustPressed(int keyCode);
            // Vector2i getMousePosition();
            //update();

        protected:
        private:
            //Vector2i _mousePosition;
            std::unordered_map<int, bool> _keyStates;

    };

#endif /* !_INPUTE_MANAGER_CORE_ */
