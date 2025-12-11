#ifndef RTYPE_CLIENT_RENDERING_SFMLRENDERER_HPP
#define RTYPE_CLIENT_RENDERING_SFMLRENDERER_HPP

#include <engine/rendering/IRenderer.hpp>
#include <client/rendering/SFMLSprite.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <memory>

namespace rtype
{
    namespace client
    {
        namespace rendering
        {

            class SFMLRenderer : public engine::rendering::IRenderer {
            private:
                sf::RenderWindow *window_;

            public:
                explicit SFMLRenderer(sf::RenderWindow *window);
                ~SFMLRenderer() override = default;

                // IRenderer implementation
                void clear() override;
                void draw(engine::rendering::ISprite &sprite, const engine::rendering::Transform &transform) override;
                void display() override;
                void setCamera(const engine::rendering::Camera &camera) override;

                // Helper: get window
                const sf::RenderWindow &getWindow() const { return *window_; }
            };

        }
    }
}

#endif // RTYPE_CLIENT_RENDERING_SFMLRENDERER_HPP
