#include <SFML/Graphics.hpp>

int main()
{
    auto window = sf::RenderWindow(sf::VideoMode({800, 600}), "R-Type");
    window.setFramerateLimit(60);

    while (window.isOpen())
    {
        while (const auto event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
            {
                window.close();
            }
        }

        window.clear();
        
        sf::CircleShape shape(50.f);
        shape.setFillColor(sf::Color::Green);
        shape.setPosition({375.f, 275.f});
        window.draw(shape);

        window.display();
    }
}
