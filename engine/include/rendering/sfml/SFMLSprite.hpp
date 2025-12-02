#ifndef RTYPE_ENGINE_RENDERING_SFML_SFMLSPRITE_HPP
#define RTYPE_ENGINE_RENDERING_SFML_SFMLSPRITE_HPP

#include <engine/include/rendering/ISprite.hpp>
#include <engine/include/rendering/sfml/SFMLTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>

namespace rtype
{
    namespace engine
    {
        namespace rendering
        {
            namespace sfml
            {

                class SFMLSprite : public ISprite {
                    public:
                        SFMLSprite() : currentTexture_(nullptr) {};
                        ~SFMLSprite() override = default;

                        // ISprite implementation
                        void setTexture(ITexture *texture) override;
                        void setPosition(Vector2f position) override;
                        void setTextureRect(IntRect rect) override;
                        // SFML-specific: get native sprite
                        const sf::Sprite &getNativeSprite() const { return sprite_; }
                        sf::Sprite &getNativeSprite() { return sprite_; }

                    private:
                        sf::Sprite sprite_;
                        SFMLTexture *currentTexture_;

                };

            }
        }
    }
}

#endif // RTYPE_ENGINE_RENDERING_SFML_SFMLSPRITE_HPP
