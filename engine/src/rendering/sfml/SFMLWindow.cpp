#include <rendering/sfml/SFMLWindow.hpp>
#include <SFML/Window/Mouse.hpp>

namespace rtype
{
    namespace engine
    {
        namespace rendering
        {
            namespace sfml
            {

                SFMLWindow::SFMLWindow() : m_width(800), m_height(600), m_title("Game Window") {}

                void SFMLWindow::create(uint32_t width, uint32_t height, const std::string &title)
                {
                    m_width = width;
                    m_height = height;
                    m_title = title;

                    m_window.create(sf::VideoMode(width, height), title);
                    m_window.setFramerateLimit(60);
                }

                void SFMLWindow::close()
                {
                    m_window.close();
                }

                bool SFMLWindow::isOpen() const
                {
                    return m_window.isOpen();
                }

                bool SFMLWindow::pollEvent(sf::Event &event)
                {
                    return m_window.pollEvent(event);
                }

                void SFMLWindow::clear()
                {
                    m_window.clear();
                }

                void SFMLWindow::display()
                {
                    m_window.display();
                }

                sf::RenderWindow &SFMLWindow::getSFMLWindow()
                {
                    return m_window;
                }

                const sf::RenderWindow &SFMLWindow::getSFMLWindow() const
                {
                    return m_window;
                }

                rtype::engine::rendering::Vector2i SFMLWindow::getMousePosition() const
                {
                    sf::Vector2i pos = sf::Mouse::getPosition(m_window);
                    return rtype::engine::rendering::Vector2i{pos.x, pos.y};
                }

                rtype::engine::rendering::Vector2u SFMLWindow::getSize() const
                {
                    return rtype::engine::rendering::Vector2u{m_width, m_height};
                }

            }
        }
    }
}
