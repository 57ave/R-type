#ifndef RTYPE_ENGINE_RENDERING_CAMERA_HPP
#define RTYPE_ENGINE_RENDERING_CAMERA_HPP

#include <engine/rendering/ISprite.hpp>

namespace rtype
{
    namespace engine
    {
        namespace rendering
        {

            class Camera {
                private:
                    Vector2f position_;
                    float zoom_;
                    IntRect viewport_;

                public:
                    Camera();
                    ~Camera() = default;

                    void setPosition(Vector2f position);
                    void setZoom(float zoom);
                    Vector2f worldToScreen(Vector2f worldPos) const;
                    Vector2f screenToWorld(Vector2f screenPos) const;
                    Vector2f getPosition() const { return position_; }
                    float getZoom() const { return zoom_; }
                    IntRect getViewport() const { return viewport_; }
                };

        }
    }
}

#endif // RTYPE_ENGINE_RENDERING_CAMERA_HPP
