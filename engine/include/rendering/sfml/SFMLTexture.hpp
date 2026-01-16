#ifndef ENG_ENGINE_RENDERING_SFML_SFMLTEXTURE_HPP
#define ENG_ENGINE_RENDERING_SFML_SFMLTEXTURE_HPP

#include <rendering/ITexture.hpp>
#include <SFML/Graphics/Texture.hpp>

namespace eng
{
    namespace engine
    {
        namespace rendering
        {
            namespace sfml
            {

                class SFMLTexture : public ITexture {
                    public:
                        SFMLTexture() = default;
                        ~SFMLTexture() override = default;

                        // ITexture implementation
                        Vector2u getSize() const override;
                        bool loadFromFile(const std::string &path) override;
                        // SFML-specific: get native texture
                        const sf::Texture &getNativeTexture() const { return texture_; }
                    private:
                        sf::Texture texture_;
                };

            }
        }
    }
}

#endif // ENG_ENGINE_RENDERING_SFML_SFMLTEXTURE_HPP
