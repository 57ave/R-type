#ifndef RTYPE_ENGINE_RENDERING_ITEXT_HPP
#define RTYPE_ENGINE_RENDERING_ITEXT_HPP

#include "Types.hpp"
#include "IFont.hpp"
#include <string>
#include <cstdint>

namespace rtype
{
    namespace engine
    {
        namespace rendering
        {

            /**
             * @brief Abstract interface for text rendering
             * 
             * This interface provides a platform-agnostic way to create and manipulate text.
             * Supports setting content, position, size, colors, and alignment.
             */
            class IText {
            public:
                enum class Alignment {
                    Left,
                    Center,
                    Right
                };

                virtual ~IText() = default;

                // Content
                virtual void setString(const std::string& text) = 0;
                virtual std::string getString() const = 0;

                // Font
                virtual void setFont(IFont* font) = 0;

                // Position and size
                virtual void setPosition(float x, float y) = 0;
                virtual void setPosition(const Vector2f& position) = 0;
                virtual Vector2f getPosition() const = 0;
                virtual void setCharacterSize(unsigned int size) = 0;
                virtual unsigned int getCharacterSize() const = 0;

                // Colors (RGBA format: 0xRRGGBBAA)
                virtual void setFillColor(uint32_t color) = 0;
                virtual uint32_t getFillColor() const = 0;
                virtual void setOutlineColor(uint32_t color) = 0;
                virtual uint32_t getOutlineColor() const = 0;
                virtual void setOutlineThickness(float thickness) = 0;
                virtual float getOutlineThickness() const = 0;

                // Bounds (for collision/hover detection)
                virtual FloatRect getLocalBounds() const = 0;
                virtual FloatRect getGlobalBounds() const = 0;

                // Alignment helper
                virtual void setAlignment(Alignment alignment) = 0;
                virtual Alignment getAlignment() const = 0;

                // Origin (for alignment)
                virtual void setOrigin(float x, float y) = 0;
                virtual Vector2f getOrigin() const = 0;

                // Style
                virtual void setStyle(uint32_t style) = 0;
                virtual uint32_t getStyle() const = 0;

                // Letter spacing
                virtual void setLetterSpacing(float spacing) = 0;
                virtual float getLetterSpacing() const = 0;
            };

            // Text styles (can be combined with bitwise OR)
            namespace TextStyle {
                constexpr uint32_t Regular       = 0;
                constexpr uint32_t Bold          = 1 << 0;
                constexpr uint32_t Italic        = 1 << 1;
                constexpr uint32_t Underlined    = 1 << 2;
                constexpr uint32_t StrikeThrough = 1 << 3;
            }

        }
    }
}

#endif // RTYPE_ENGINE_RENDERING_ITEXT_HPP
