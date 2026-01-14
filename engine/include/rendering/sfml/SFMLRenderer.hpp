#ifndef ENG_ENGINE_RENDERING_SFML_SFMLRENDERER_HPP
#define ENG_ENGINE_RENDERING_SFML_SFMLRENDERER_HPP

#include <rendering/IRenderer.hpp>
#include <rendering/sfml/SFMLSprite.hpp>
#include <rendering/sfml/SFMLText.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <memory>

namespace eng
{
    namespace engine
    {
        namespace rendering
        {
            namespace sfml
            {

                class SFMLRenderer : public IRenderer {
                    public:
                        explicit SFMLRenderer(sf::RenderWindow *window);
                        ~SFMLRenderer() override = default;

                        // IRenderer implementation
                        void clear() override;
                        void draw(ISprite &sprite, const Transform &transform) override;
                        void drawText(IText &text) override;
                        void drawRect(const FloatRect &rect, uint32_t fillColor, uint32_t outlineColor = 0, float outlineThickness = 0.0f) override;
                        void display() override;
                        void setCamera(const Camera &camera) override;
                        // Helper: get window
                        const sf::RenderWindow &getWindow() const { return *window_; }

                    private:
                        sf::RenderWindow *window_;

                        // Helper to convert RGBA uint32 to sf::Color
                        static sf::Color toSFMLColor(uint32_t rgba);
                };

            }
        }
    }
}

#endif // ENG_ENGINE_RENDERING_SFML_SFMLRENDERER_HPP
