#include <rendering/sfml/SFMLTexture.hpp>

namespace rtype
{
    namespace engine
    {
        namespace rendering
        {
            namespace sfml
            {

                Vector2u SFMLTexture::getSize() const
                {
                    auto size = texture_.getSize();
                    return Vector2u(size.x, size.y);
                }

                bool SFMLTexture::loadFromFile(const std::string &path)
                {
                    return texture_.loadFromFile(path);
                }

                void SFMLTexture::loadFromImage(const sf::Image &image)
                {
                    texture_.loadFromImage(image);
                }

            }
        }
    }
}
