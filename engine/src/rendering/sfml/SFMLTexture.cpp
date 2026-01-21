#include <rendering/sfml/SFMLTexture.hpp>

namespace eng {
namespace engine {
namespace rendering {
namespace sfml {

Vector2u SFMLTexture::getSize() const {
    auto size = texture_.getSize();
    return Vector2u(size.x, size.y);
}

bool SFMLTexture::loadFromFile(const std::string& path) {
    return texture_.loadFromFile(path);
}

bool SFMLTexture::loadFromImage(const sf::Image& image, const sf::IntRect& area) {
    return texture_.loadFromImage(image, area);
}

}  // namespace sfml
}  // namespace rendering
}  // namespace engine
}  // namespace eng
