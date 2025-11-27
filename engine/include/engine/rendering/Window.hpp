#ifndef RTYPE_ENGINE_RENDERING_WINDOW_HPP
#define RTYPE_ENGINE_RENDERING_WINDOW_HPP

#include <string>
#include <cstdint>
#include <engine/rendering/ITexture.hpp>

namespace rtype
{
    namespace engine
    {
        namespace rendering
        {
            class Window {
                private:
                    uint32_t width_;
                    uint32_t height_;
                    std::string title_;
                    bool isOpen_;

                public:
                    Window();
                    ~Window() = default;

                    void create(uint32_t width, uint32_t height, const std::string &title);
                    void close();
                    // Vector<Event> pollEvents(); // Event type not defined in UML
                    Vector2u getSize() const;

                    // Getters
                    uint32_t getWidth() const { return width_; }
                    uint32_t getHeight() const { return height_; }
                    std::string getTitle() const { return title_; }
                    bool isOpen() const { return isOpen_; }
            };

        }
    }
}

#endif // RTYPE_ENGINE_RENDERING_WINDOW_HPP
