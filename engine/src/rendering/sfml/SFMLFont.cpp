#include <rendering/sfml/SFMLFont.hpp>
#include "core/Logger.hpp"

namespace eng
{
    namespace engine
    {
        namespace rendering
        {
            namespace sfml
            {

                bool SFMLFont::loadFromFile(const std::string& filename)
                {
                    m_loaded = m_font.loadFromFile(filename);
                    if (!m_loaded) {
                        LOG_ERROR("SFMLFONT", "Failed to load font from: " + filename);
                    }
                    return m_loaded;
                }

                bool SFMLFont::isLoaded() const
                {
                    return m_loaded;
                }

            }
        }
    }
}
