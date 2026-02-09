#ifndef ENG_ENGINE_RENDERING_ITEXTURE_HPP
#define ENG_ENGINE_RENDERING_ITEXTURE_HPP

#include <string>
#include "rendering/Types.hpp"

namespace eng
{
    namespace engine
    {
        namespace rendering
        {
            class ITexture {
                public:
                    virtual ~ITexture() = default;

                    virtual Vector2u getSize() const = 0;
                    virtual bool loadFromFile(const std::string &path) = 0;
            };

        }
    }
}

#endif // ENG_ENGINE_RENDERING_ITEXTURE_HPP
