#include <rendering/sfml/SFMLWindow.hpp>
#include <SFML/Window/Mouse.hpp>

namespace eng
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

                bool SFMLWindow::pollEvent(eng::engine::InputEvent &event)
                {
                    sf::Event sfEvent;
                    if (!m_window.pollEvent(sfEvent)) {
                        return false;
                    }

                    // Convert SFML event to engine event
                    switch (sfEvent.type) {
                        case sf::Event::Closed:
                            event.type = eng::engine::EventType::Closed;
                            break;
                        case sf::Event::Resized:
                            event.type = eng::engine::EventType::Resized;
                            event.size.width = sfEvent.size.width;
                            event.size.height = sfEvent.size.height;
                            break;
                        case sf::Event::LostFocus:
                            event.type = eng::engine::EventType::LostFocus;
                            break;
                        case sf::Event::GainedFocus:
                            event.type = eng::engine::EventType::GainedFocus;
                            break;
                        case sf::Event::KeyPressed:
                            event.type = eng::engine::EventType::KeyPressed;
                            event.key.code = eng::engine::internal::sfmlKeyToEngineKey(sfEvent.key.code);
                            event.key.alt = sfEvent.key.alt;
                            event.key.control = sfEvent.key.control;
                            event.key.shift = sfEvent.key.shift;
                            event.key.system = sfEvent.key.system;
                            break;
                        case sf::Event::KeyReleased:
                            event.type = eng::engine::EventType::KeyReleased;
                            event.key.code = eng::engine::internal::sfmlKeyToEngineKey(sfEvent.key.code);
                            event.key.alt = sfEvent.key.alt;
                            event.key.control = sfEvent.key.control;
                            event.key.shift = sfEvent.key.shift;
                            event.key.system = sfEvent.key.system;
                            break;
                        case sf::Event::MouseMoved:
                            event.type = eng::engine::EventType::MouseMoved;
                            event.mouseMove.x = sfEvent.mouseMove.x;
                            event.mouseMove.y = sfEvent.mouseMove.y;
                            break;
                        case sf::Event::MouseButtonPressed:
                            event.type = eng::engine::EventType::MouseButtonPressed;
                            event.mouseButton.button = sfEvent.mouseButton.button;
                            event.mouseButton.x = sfEvent.mouseButton.x;
                            event.mouseButton.y = sfEvent.mouseButton.y;
                            break;
                        case sf::Event::MouseButtonReleased:
                            event.type = eng::engine::EventType::MouseButtonReleased;
                            event.mouseButton.button = sfEvent.mouseButton.button;
                            event.mouseButton.x = sfEvent.mouseButton.x;
                            event.mouseButton.y = sfEvent.mouseButton.y;
                            break;
                        case sf::Event::MouseWheelScrolled:
                            event.type = eng::engine::EventType::MouseWheelScrolled;
                            event.mouseWheelScroll.delta = sfEvent.mouseWheelScroll.delta;
                            event.mouseWheelScroll.x = sfEvent.mouseWheelScroll.x;
                            event.mouseWheelScroll.y = sfEvent.mouseWheelScroll.y;
                            break;
                        default:
                            // Unknown event type, skip
                            return pollEvent(event); // Try next event
                    }

                    return true;
                }

                bool SFMLWindow::pollEventSFML(sf::Event &event)
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

                eng::engine::rendering::Vector2i SFMLWindow::getMousePosition() const
                {
                    sf::Vector2i pos = sf::Mouse::getPosition(m_window);
                    return eng::engine::rendering::Vector2i{pos.x, pos.y};
                }

                eng::engine::rendering::Vector2u SFMLWindow::getSize() const
                {
                    return eng::engine::rendering::Vector2u{m_width, m_height};
                }

            }
        }
    }
}
