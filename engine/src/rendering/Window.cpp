#include <engine/rendering/Window.hpp>

namespace rtype
{
    namespace engine
    {
        namespace rendering
        {

            Window::Window() : width_(800), height_(600), title_("Game Window"), isOpen_(false) {}

            void Window::create(uint32_t width, uint32_t height, const std::string &title)
            {
                width_ = width;
                height_ = height;
                title_ = title;
                isOpen_ = true;
            }

            void Window::close()
            {
                isOpen_ = false;
            }

            Vector2u Window::getSize() const
            {
                return Vector2u(width_, height_);
            }

        }
    }
}
