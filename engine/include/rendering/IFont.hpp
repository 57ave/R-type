#ifndef ENG_ENGINE_RENDERING_IFONT_HPP
#define ENG_ENGINE_RENDERING_IFONT_HPP

#include <string>

namespace eng {
namespace engine {
namespace rendering {

/**
 * @brief Abstract interface for fonts
 *
 * This interface provides a platform-agnostic way to load and use fonts.
 * Implementations should wrap platform-specific font loading (SFML, SDL, etc.)
 */
class IFont {
public:
    virtual ~IFont() = default;

    /**
     * @brief Load a font from a file
     * @param filename Path to the font file (.ttf, .otf, etc.)
     * @return true if loading succeeded, false otherwise
     */
    virtual bool loadFromFile(const std::string& filename) = 0;

    /**
     * @brief Check if the font is loaded and valid
     * @return true if the font is ready to use
     */
    virtual bool isLoaded() const = 0;
};

}  // namespace rendering
}  // namespace engine
}  // namespace eng

#endif  // ENG_ENGINE_RENDERING_IFONT_HPP
