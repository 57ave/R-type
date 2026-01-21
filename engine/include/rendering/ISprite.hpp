#ifndef ENG_ENGINE_RENDERING_ISPRITE_HPP
#define ENG_ENGINE_RENDERING_ISPRITE_HPP

#include <rendering/ITexture.hpp>

namespace eng {
namespace engine {
namespace rendering {
class ISprite {
public:
    virtual ~ISprite() = default;

    virtual void setTexture(ITexture* texture) = 0;
    virtual void setPosition(Vector2f position) = 0;
    virtual void setTextureRect(IntRect rect) = 0;
};

}  // namespace rendering
}  // namespace engine
}  // namespace eng

#endif  // ENG_ENGINE_RENDERING_ISPRITE_HPP
