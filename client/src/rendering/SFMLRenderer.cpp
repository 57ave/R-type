#include <client/rendering/SFMLRenderer.hpp>
#include <SFML/Graphics/View.hpp>

namespace rtype
{
    namespace client
    {
        namespace rendering
        {

            SFMLRenderer::SFMLRenderer(sf::RenderWindow *window) : window_(window) {}

            void SFMLRenderer::clear()
            {
                window_->clear(sf::Color::Black);
            }

            void SFMLRenderer::draw(engine::rendering::ISprite &sprite, const engine::rendering::Transform &transform)
            {
                SFMLSprite *sfmlSprite = dynamic_cast<SFMLSprite *>(&sprite);
                if (sfmlSprite) {
                    sf::Sprite &nativeSprite = sfmlSprite->getNativeSprite();

                    // Apply transform
                    nativeSprite.setPosition(transform.position.x, transform.position.y);
                    nativeSprite.setRotation(transform.rotation);
                    nativeSprite.setScale(transform.scale.x, transform.scale.y);

                    window_->draw(nativeSprite);
                }
            }

            void SFMLRenderer::display()
            {
                window_->display();
            }

            void SFMLRenderer::setCamera(const engine::rendering::Camera &camera)
            {
                engine::rendering::Vector2f pos = camera.getPosition();
                float zoom = camera.getZoom();
                engine::rendering::IntRect viewport = camera.getViewport();

                sf::View view;
                view.setCenter(pos.x, pos.y);
                view.setSize(viewport.width / zoom, viewport.height / zoom);
                view.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));

                window_->setView(view);
            }

        }
    }
}