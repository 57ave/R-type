#ifndef RTYPE_CLIENT_RENDERING_SFMLSPRITE_HPP
#define RTYPE_CLIENT_RENDERING_SFMLSPRITE_HPP

#include <engine/rendering/ISprite.hpp>
#include <client/rendering/SFMLTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace rtype
{
    namespace client
    {
        namespace rendering
        {

            class SFMLSprite : public engine::rendering::ISprite {
            private:
                sf::Sprite sprite_;
                SFMLTexture *currentTexture_;

            public:
                SFMLSprite();
                ~SFMLSprite() override = default;

                // ISprite implementation
                void setTexture(engine::rendering::ITexture *texture) override;
                void setPosition(engine::rendering::Vector2f position) override;
                void setTextureRect(engine::rendering::IntRect rect) override;

                // SFML-specific: get native sprite
                const sf::Sprite &getNativeSprite() const { return sprite_; }
                sf::Sprite &getNativeSprite() { return sprite_; }
            };

        }
    }
}

#endif // RTYPE_CLIENT_RENDERING_SFMLSPRITE_HPP
