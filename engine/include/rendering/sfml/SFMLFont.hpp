#ifndef ENG_ENGINE_RENDERING_SFML_SFMLFONT_HPP
#define ENG_ENGINE_RENDERING_SFML_SFMLFONT_HPP

#include <SFML/Graphics/Font.hpp>
#include <rendering/IFont.hpp>
#include <string>

namespace eng {
namespace engine {
namespace rendering {
namespace sfml {

/**
 * @brief SFML implementation of the IFont interface
 *
 * Wraps sf::Font to provide font loading and management
 */
class SFMLFont : public IFont {
public:
    SFMLFont() = default;
    ~SFMLFont() override = default;

    // IFont implementation
    bool loadFromFile(const std::string& filename) override;
    bool isLoaded() const override;

    // Access to native SFML font (for SFMLText)
    sf::Font& getNativeFont() { return m_font; }
    const sf::Font& getNativeFont() const { return m_font; }

private:
    sf::Font m_font;
    bool m_loaded = false;
};

}  // namespace sfml
}  // namespace rendering
}  // namespace engine
}  // namespace eng

#endif  // ENG_ENGINE_RENDERING_SFML_SFMLFONT_HPP
