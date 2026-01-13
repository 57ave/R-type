#ifndef RTYPE_ENGINE_RENDERING_IRENDERER_HPP
#define RTYPE_ENGINE_RENDERING_IRENDERER_HPP

#include <rendering/ISprite.hpp>
#include <rendering/IText.hpp>
#include <rendering/Camera.hpp>
#include "rendering/Types.hpp"

namespace rtype
{
    namespace engine
    {
        namespace rendering
        {

            class IRenderer {
                public:
                    virtual ~IRenderer() = default;

                    virtual void clear() = 0;
                    virtual void draw(ISprite &sprite, const Transform &transform) = 0;
                    virtual void drawText(IText &text) = 0;
                    virtual void drawRect(const FloatRect &rect, uint32_t fillColor, uint32_t outlineColor = 0, float outlineThickness = 0.0f) = 0;
                    virtual void display() = 0;
                    virtual void setCamera(const Camera &camera) = 0;
            };

        }
    }
}

#endif // RTYPE_ENGINE_RENDERING_IRENDERER_HPP
