#include <client/rendering/SFMLTexture.hpp>

namespace rtype
{
    namespace client
    {
        namespace rendering
        {

            engine::rendering::Vector2u SFMLTexture::getSize() const
            {
                auto size = texture_.getSize();
                return engine::rendering::Vector2u(size.x, size.y);
            }

            bool SFMLTexture::loadFromFile(const std::string &path)
            {
                return texture_.loadFromFile(path);
            }

        }
    }
}
