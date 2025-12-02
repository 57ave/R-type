#ifndef RTYPE_ENGINE_RENDERING_SFML_SFMLRENDERER_HPP
#define RTYPE_ENGINE_RENDERING_SFML_SFMLRENDERER_HPP

#include <engine/rendering/IRenderer.hpp>
#include <engine/rendering/sfml/SFMLSprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>

namespace rtype
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
                        void display() override;
                        void setCamera(const Camera &camera) override;
                        // Helper: get window
                        const sf::RenderWindow &getWindow() const { return *window_; }

                    private:
                        sf::RenderWindow *window_;

                };

            }
        }
    }
}

#endif // RTYPE_ENGINE_RENDERING_SFML_SFMLRENDERER_HPP
