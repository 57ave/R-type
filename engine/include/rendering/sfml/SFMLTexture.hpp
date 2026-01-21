#ifndef ENG_ENGINE_RENDERING_SFML_SFMLTEXTURE_HPP
#define ENG_ENGINE_RENDERING_SFML_SFMLTEXTURE_HPP

#include <SFML/Graphics/Texture.hpp>
#include <rendering/ITexture.hpp>

namespace eng {
namespace engine {
namespace rendering {
namespace sfml {

class SFMLTexture : public ITexture {
public:
    SFMLTexture() = default;
    ~SFMLTexture() override = default;

    Vector2u getSize() const override;
    bool loadFromFile(const std::string& path) override;
    bool loadFromImage(const sf::Image& image, const sf::IntRect& area = sf::IntRect());
    const sf::Texture& getNativeTexture() const { return texture_; }

private:
    sf::Texture texture_;
};

}  // namespace sfml
}  // namespace rendering
}  // namespace engine
}  // namespace eng

#endif  // ENG_ENGINE_RENDERING_SFML_SFMLTEXTURE_HPP
