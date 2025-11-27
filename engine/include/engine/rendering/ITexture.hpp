#ifndef RTYPE_ENGINE_RENDERING_ITEXTURE_HPP
#define RTYPE_ENGINE_RENDERING_ITEXTURE_HPP

#include <string>
#include <cstdint>

namespace rtype
{
    namespace engine
    {
        namespace rendering
        {
            struct Vector2u {
                uint32_t x;
                uint32_t y;

                Vector2u() : x(0), y(0) {}
                Vector2u(uint32_t x, uint32_t y) : x(x), y(y) {}
            };

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
