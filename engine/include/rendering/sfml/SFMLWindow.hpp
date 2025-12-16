#ifndef RTYPE_ENGINE_RENDERING_SFML_SFMLWINDOW_HPP
#define RTYPE_ENGINE_RENDERING_SFML_SFMLWINDOW_HPP

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <rendering/Types.hpp>
#include <string>

namespace rtype
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
                        void close();
                        bool isOpen() const;

                        // Event handling
                        bool pollEvent(sf::Event &event);

                        // Rendering
                        void clear();
                        void display();

                        // Input
                        rtype::engine::rendering::Vector2i getMousePosition() const;

                        // Access
                        sf::RenderWindow &getSFMLWindow();
                        const sf::RenderWindow &getSFMLWindow() const;

                        // Window info
                        rtype::engine::rendering::Vector2u getSize() const;

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

#endif // RTYPE_ENGINE_RENDERING_SFML_SFMLWINDOW_HPP
