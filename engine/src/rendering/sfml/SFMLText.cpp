#include <rendering/sfml/SFMLText.hpp>

namespace eng
{
    namespace engine
    {
        namespace rendering
        {
            namespace sfml
            {

                SFMLText::SFMLText()
                {
                    // Default values
                    m_text.setFillColor(sf::Color::White);
                    m_text.setCharacterSize(24);
                }

                void SFMLText::setString(const std::string& text)
                {
                    m_text.setString(text);
                    updateOriginForAlignment();
                }

                std::string SFMLText::getString() const
                {
                    return m_text.getString();
                }

                void SFMLText::setFont(IFont* font)
                {
                    SFMLFont* sfmlFont = dynamic_cast<SFMLFont*>(font);
                    if (sfmlFont && sfmlFont->isLoaded()) {
                        m_fontRef = sfmlFont;
                        m_text.setFont(sfmlFont->getNativeFont());
                        updateOriginForAlignment();
                    }
                }

                void SFMLText::setPosition(float x, float y)
                {
                    m_text.setPosition(x, y);
                }

                void SFMLText::setPosition(const Vector2f& position)
                {
                    m_text.setPosition(position.x, position.y);
                }

                Vector2f SFMLText::getPosition() const
                {
                    sf::Vector2f pos = m_text.getPosition();
                    return Vector2f(pos.x, pos.y);
                }

                void SFMLText::setCharacterSize(unsigned int size)
                {
                    m_text.setCharacterSize(size);
                    updateOriginForAlignment();
                }

                unsigned int SFMLText::getCharacterSize() const
                {
                    return m_text.getCharacterSize();
                }

                void SFMLText::setFillColor(uint32_t color)
                {
                    m_text.setFillColor(toSFMLColor(color));
                }

                uint32_t SFMLText::getFillColor() const
                {
                    return fromSFMLColor(m_text.getFillColor());
                }

                void SFMLText::setOutlineColor(uint32_t color)
                {
                    m_text.setOutlineColor(toSFMLColor(color));
                }

                uint32_t SFMLText::getOutlineColor() const
                {
                    return fromSFMLColor(m_text.getOutlineColor());
                }

                void SFMLText::setOutlineThickness(float thickness)
                {
                    m_text.setOutlineThickness(thickness);
                }

                float SFMLText::getOutlineThickness() const
                {
                    return m_text.getOutlineThickness();
                }

                FloatRect SFMLText::getLocalBounds() const
                {
                    sf::FloatRect bounds = m_text.getLocalBounds();
                    return FloatRect(bounds.left, bounds.top, bounds.width, bounds.height);
                }

                FloatRect SFMLText::getGlobalBounds() const
                {
                    sf::FloatRect bounds = m_text.getGlobalBounds();
                    return FloatRect(bounds.left, bounds.top, bounds.width, bounds.height);
                }

                void SFMLText::setAlignment(Alignment alignment)
                {
                    m_alignment = alignment;
                    updateOriginForAlignment();
                }

                IText::Alignment SFMLText::getAlignment() const
                {
                    return m_alignment;
                }

                void SFMLText::setOrigin(float x, float y)
                {
                    m_text.setOrigin(x, y);
                }

                Vector2f SFMLText::getOrigin() const
                {
                    sf::Vector2f origin = m_text.getOrigin();
                    return Vector2f(origin.x, origin.y);
                }

                void SFMLText::setStyle(uint32_t style)
                {
                    m_text.setStyle(style);
                }

                uint32_t SFMLText::getStyle() const
                {
                    return m_text.getStyle();
                }

                void SFMLText::setLetterSpacing(float spacing)
                {
                    m_text.setLetterSpacing(spacing);
                }

                float SFMLText::getLetterSpacing() const
                {
                    return m_text.getLetterSpacing();
                }

                sf::Color SFMLText::toSFMLColor(uint32_t rgba)
                {
                    // Format: 0xRRGGBBAA
                    uint8_t r = (rgba >> 24) & 0xFF;
                    uint8_t g = (rgba >> 16) & 0xFF;
                    uint8_t b = (rgba >> 8) & 0xFF;
                    uint8_t a = rgba & 0xFF;
                    return sf::Color(r, g, b, a);
                }

                uint32_t SFMLText::fromSFMLColor(const sf::Color& color)
                {
                    return (static_cast<uint32_t>(color.r) << 24) |
                           (static_cast<uint32_t>(color.g) << 16) |
                           (static_cast<uint32_t>(color.b) << 8) |
                           static_cast<uint32_t>(color.a);
                }

                void SFMLText::updateOriginForAlignment()
                {
                    sf::FloatRect bounds = m_text.getLocalBounds();
                    
                    switch (m_alignment) {
                        case Alignment::Left:
                            m_text.setOrigin(bounds.left, bounds.top);
                            break;
                        case Alignment::Center:
                            m_text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top);
                            break;
                        case Alignment::Right:
                            m_text.setOrigin(bounds.left + bounds.width, bounds.top);
                            break;
                    }
                }

            }
        }
    }
}
