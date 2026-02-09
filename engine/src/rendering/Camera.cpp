#include <rendering/Camera.hpp>

namespace eng
{
    namespace engine
    {
        namespace rendering
        {

            Camera::Camera() : position_(0.0f, 0.0f), zoom_(1.0f), viewport_(0, 0, 800, 600) {}

            void Camera::setPosition(Vector2f position)
            {
                position_ = position;
            }

            void Camera::setZoom(float zoom)
            {
                if (zoom > 0.0f)
                    zoom_ = zoom;
            }

            Vector2f Camera::worldToScreen(Vector2f worldPos) const
            {
                Vector2f relative;
                relative.x = (worldPos.x - position_.x) * zoom_;
                relative.y = (worldPos.y - position_.y) * zoom_;

                Vector2f screenPos;
                screenPos.x = relative.x + (viewport_.width / 2.0f);
                screenPos.y = relative.y + (viewport_.height / 2.0f);

                return screenPos;
            }

            Vector2f Camera::screenToWorld(Vector2f screenPos) const
            {
                Vector2f relative;
                relative.x = (screenPos.x - (viewport_.width / 2.0f)) / zoom_;
                relative.y = (screenPos.y - (viewport_.height / 2.0f)) / zoom_;

                Vector2f worldPos;
                worldPos.x = relative.x + position_.x;
                worldPos.y = relative.y + position_.y;

                return worldPos;
            }

        }
    }
}
