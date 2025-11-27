#ifndef RTYPE_ENGINE_RENDERING_IRENDERER_HPP
#define RTYPE_ENGINE_RENDERING_IRENDERER_HPP

#include <engine/rendering/ISprite.hpp>
#include <engine/rendering/Camera.hpp>

namespace rtype
{
    namespace engine
    {
        namespace rendering
        {

            struct Transform {
                Vector2f position;
                float rotation;
                Vector2f scale;
                Transform() : position(0.0f, 0.0f), rotation(0.0f), scale(1.0f, 1.0f){}
            };

            class IRenderer {
                public:
                    virtual ~IRenderer() = default;

                    virtual void clear() = 0;
                    virtual void draw(ISprite &sprite, const Transform &transform) = 0;
                    virtual void present() = 0;
                    virtual void setCamera(const Camera &camera) = 0;
            };

        }
    }
}

#endif // RTYPE_ENGINE_RENDERING_IRENDERER_HPP
