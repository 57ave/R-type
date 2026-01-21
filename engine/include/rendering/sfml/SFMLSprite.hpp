#ifndef ENG_ENGINE_RENDERING_SFML_SFMLSPRITE_HPP
#define ENG_ENGINE_RENDERING_SFML_SFMLSPRITE_HPP

#include <SFML/Graphics/Sprite.hpp>
#include <rendering/ISprite.hpp>
#include <rendering/sfml/SFMLTexture.hpp>

namespace eng {
namespace engine {
namespace rendering {
namespace sfml {

class SFMLSprite : public ISprite {
public:
    SFMLSprite();
    ~SFMLSprite() override;

    // ISprite implementation
    void setTexture(ITexture* texture) override;
    void setPosition(Vector2f position) override;
    void setTextureRect(IntRect rect) override;
    // SFML-specific: get native sprite
    const sf::Sprite& getNativeSprite() const { return sprite_; }
    sf::Sprite& getNativeSprite() { return sprite_; }

private:
    sf::Sprite sprite_;
    SFMLTexture* currentTexture_;
};

}  // namespace sfml
}  // namespace rendering
}  // namespace engine
}  // namespace eng

#endif  // ENG_ENGINE_RENDERING_SFML_SFMLSPRITE_HPP
