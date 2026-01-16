#ifndef ENG_ENGINE_RENDERING_SFML_SFMLTEXT_HPP
#define ENG_ENGINE_RENDERING_SFML_SFMLTEXT_HPP

#include <rendering/IText.hpp>
#include <rendering/sfml/SFMLFont.hpp>
#include <SFML/Graphics/Text.hpp>
#include <string>

namespace eng
{
    namespace engine
    {
        namespace rendering
        {
            namespace sfml
            {

                /**
                 * @brief SFML implementation of the IText interface
                 * 
                 * Wraps sf::Text to provide text rendering functionality
                 */
                class SFMLText : public IText {
                public:
                    SFMLText();
                    ~SFMLText() override = default;

                    // IText implementation - Content
                    void setString(const std::string& text) override;
                    std::string getString() const override;

                    // Font
                    void setFont(IFont* font) override;

                    // Position and size
                    void setPosition(float x, float y) override;
                    void setPosition(const Vector2f& position) override;
                    Vector2f getPosition() const override;
                    void setCharacterSize(unsigned int size) override;
                    unsigned int getCharacterSize() const override;

                    // Colors
                    void setFillColor(uint32_t color) override;
                    uint32_t getFillColor() const override;
                    void setOutlineColor(uint32_t color) override;
                    uint32_t getOutlineColor() const override;
                    void setOutlineThickness(float thickness) override;
                    float getOutlineThickness() const override;

                    // Bounds
                    FloatRect getLocalBounds() const override;
                    FloatRect getGlobalBounds() const override;

                    // Alignment
                    void setAlignment(Alignment alignment) override;
                    Alignment getAlignment() const override;

                    // Origin
                    void setOrigin(float x, float y) override;
                    Vector2f getOrigin() const override;

                    // Style
                    void setStyle(uint32_t style) override;
                    uint32_t getStyle() const override;

                    // Letter spacing
                    void setLetterSpacing(float spacing) override;
                    float getLetterSpacing() const override;

                    // Access to native SFML text (for renderer)
                    sf::Text& getNativeText() { return m_text; }
                    const sf::Text& getNativeText() const { return m_text; }

                private:
                    sf::Text m_text;
                    Alignment m_alignment = Alignment::Left;
                    SFMLFont* m_fontRef = nullptr;

                    // Helper to convert RGBA uint32 to sf::Color
                    static sf::Color toSFMLColor(uint32_t rgba);
                    // Helper to convert sf::Color to RGBA uint32
                    static uint32_t fromSFMLColor(const sf::Color& color);

                    // Update origin based on alignment
                    void updateOriginForAlignment();
                };

            }
        }
    }
}

#endif // ENG_ENGINE_RENDERING_SFML_SFMLTEXT_HPP
