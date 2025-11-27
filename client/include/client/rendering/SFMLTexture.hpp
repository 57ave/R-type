#ifndef RTYPE_CLIENT_RENDERING_SFMLTEXTURE_HPP
#define RTYPE_CLIENT_RENDERING_SFMLTEXTURE_HPP

#include <engine/rendering/ITexture.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace rtype
{
    namespace client
    {
        namespace rendering
        {

            class SFMLTexture : public engine::rendering::ITexture {
            private:
                sf::Texture texture_;

            public:
                SFMLTexture() = default;
                ~SFMLTexture() override = default;

                // ITexture implementation
                engine::rendering::Vector2u getSize() const override;
                bool loadFromFile(const std::string &path) override;

                // SFML-specific: get native texture
                const sf::Texture &getNativeTexture() const { return texture_; }
            };

        }
    }
}

#endif // RTYPE_CLIENT_RENDERING_SFMLTEXTURE_HPP
