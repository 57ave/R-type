#ifndef RTYPE_ENGINE_RENDERING_ISPRITE_HPP
#define RTYPE_ENGINE_RENDERING_ISPRITE_HPP

#include <engine/rendering/ITexture.hpp>

namespace rtype
{
    namespace engine
    {
        namespace rendering
        {
            struct Vector2f {
                float x;
                float y;

                Vector2f() : x(0.0f), y(0.0f) {}
                Vector2f(float x, float y) : x(x), y(y) {}
            };

            struct IntRect {
                int left;
                int top;
                int width;
                int height;

                IntRect() : left(0), top(0), width(0), height(0) {}
                IntRect(int left, int top, int width, int height) : left(left), top(top), width(width), height(height) {}
            };

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
