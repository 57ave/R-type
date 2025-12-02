#ifndef RTYPE_ENGINE_RENDERING_ISPRITE_HPP
#define RTYPE_ENGINE_RENDERING_ISPRITE_HPP

#include <engine/rendering/ITexture.hpp>

namespace rtype
{
    namespace engine
    {
        namespace rendering
        {
            class ISprite {
                public:
                    virtual ~ISprite() = default;

                    virtual void setTexture(ITexture *texture) = 0;
                    virtual void setPosition(Vector2f position) = 0;
                    virtual void setTextureRect(IntRect rect) = 0;
            };

        }
    }
}

#endif // RTYPE_ENGINE_RENDERING_ISPRITE_HPP
