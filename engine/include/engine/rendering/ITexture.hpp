#ifndef RTYPE_ENGINE_RENDERING_ITEXTURE_HPP
#define RTYPE_ENGINE_RENDERING_ITEXTURE_HPP

#include <string>
#include "engine/rendering/Types.hpp"

namespace rtype
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

#endif // RTYPE_ENGINE_RENDERING_ITEXTURE_HPP
