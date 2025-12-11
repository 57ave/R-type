#include <engine/include/rendering/sfml/SFMLSprite.hpp>

namespace rtype
{
    namespace engine
    {
        namespace rendering
        {
            namespace sfml
            {

                void SFMLSprite::setTexture(ITexture *texture)
                {
                    if (!texture)
                        return;

                    currentTexture_ = dynamic_cast<SFMLTexture *>(texture);
                    if (currentTexture_)
                        sprite_.setTexture(currentTexture_->getNativeTexture());
                }

                void SFMLSprite::setPosition(Vector2f position)
                {
                    sprite_.setPosition(position.x, position.y);
                }

                void SFMLSprite::setTextureRect(IntRect rect)
                {
                    sprite_.setTextureRect(sf::IntRect(rect.left, rect.top, rect.width, rect.height));
                }

            }
        }
    }
}
