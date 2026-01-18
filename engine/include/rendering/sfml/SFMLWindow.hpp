#ifndef ENG_ENGINE_RENDERING_SFML_SFMLWINDOW_HPP
#define ENG_ENGINE_RENDERING_SFML_SFMLWINDOW_HPP

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <rendering/Types.hpp>
#include <engine/Input.hpp>
#include <string>

namespace eng
{
    namespace engine
    {
        namespace rendering
        {
            namespace sfml
            {

                class SFMLWindow {
                    public:
                        SFMLWindow();
                        ~SFMLWindow() = default;

                        // Window management
                        void create(uint32_t width, uint32_t height, const std::string &title);
                        void setSize(uint32_t width, uint32_t height);
                        void setFullscreen(bool fullscreen);
                        void close();
                        bool isOpen() const;

                        // Event handling (abstracted)
                        bool pollEvent(eng::engine::InputEvent &event);
                        
                        // Event handling (SFML direct - for compatibility)
                        bool pollEventSFML(sf::Event &event);

                        // Rendering
                        void clear();
                        void display();

                        // Input
                        eng::engine::rendering::Vector2i getMousePosition() const;

                        // Access
                        sf::RenderWindow &getSFMLWindow();
                        const sf::RenderWindow &getSFMLWindow() const;

                        // Window info
                        eng::engine::rendering::Vector2u getSize() const;

                    private:
                        sf::RenderWindow m_window;
                        uint32_t m_width;
                        uint32_t m_height;
                        std::string m_title;
                };

            }
        }
    }
}

#endif // ENG_ENGINE_RENDERING_SFML_SFMLWINDOW_HPP
