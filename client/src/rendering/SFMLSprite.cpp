#include <client/rendering/SFMLSprite.hpp>

namespace rtype
{
    namespace client
    {
        namespace rendering
        {
            void SFMLSprite::setTexture(engine::rendering::ITexture *texture)
            {
                if (!texture)
                    return;

                currentTexture_ = dynamic_cast<SFMLTexture *>(texture);
                if (currentTexture_)
                    sprite_.setTexture(currentTexture_->getNativeTexture());
            }

            void SFMLSprite::setPosition(engine::rendering::Vector2f position)
            {
                sprite_.setPosition(position.x, position.y);
            }

            void SFMLSprite::setTextureRect(engine::rendering::IntRect rect)
            {
                sprite_.setTextureRect(sf::IntRect(rect.left, rect.top, rect.width, rect.height));
            }

        }
    }
}
