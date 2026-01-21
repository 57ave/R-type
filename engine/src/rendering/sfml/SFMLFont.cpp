#include <iostream>
#include <rendering/sfml/SFMLFont.hpp>

namespace eng {
namespace engine {
namespace rendering {
namespace sfml {

bool SFMLFont::loadFromFile(const std::string& filename) {
    m_loaded = m_font.loadFromFile(filename);
    if (!m_loaded) {
        std::cerr << "[SFMLFont] Failed to load font from: " << filename << std::endl;
    }
    return m_loaded;
}

bool SFMLFont::isLoaded() const {
    return m_loaded;
}

}  // namespace sfml
}  // namespace rendering
}  // namespace engine
}  // namespace eng
