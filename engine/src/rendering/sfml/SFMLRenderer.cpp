#include <SFML/Graphics/View.hpp>
#include <rendering/sfml/SFMLRenderer.hpp>

namespace eng {
namespace engine {
namespace rendering {
namespace sfml {

SFMLRenderer::SFMLRenderer(sf::RenderWindow* window) : window_(window) {}

void SFMLRenderer::clear() {
    window_->clear(sf::Color::Black);
}

void SFMLRenderer::draw(ISprite& sprite, const Transform& transform) {
    SFMLSprite* sfmlSprite = dynamic_cast<SFMLSprite*>(&sprite);
    if (sfmlSprite) {
        sf::Sprite& nativeSprite = sfmlSprite->getNativeSprite();

        // Apply transform
        nativeSprite.setPosition(transform.position.x, transform.position.y);
        nativeSprite.setRotation(transform.rotation);
        nativeSprite.setScale(transform.scale.x, transform.scale.y);

        window_->draw(nativeSprite);
    }
}

void SFMLRenderer::drawText(IText& text) {
    SFMLText* sfmlText = dynamic_cast<SFMLText*>(&text);
    if (sfmlText) {
        window_->draw(sfmlText->getNativeText());
    }
}

void SFMLRenderer::drawRect(const FloatRect& rect, uint32_t fillColor, uint32_t outlineColor,
                            float outlineThickness) {
    sf::RectangleShape shape;
    shape.setPosition(rect.left, rect.top);
    shape.setSize(sf::Vector2f(rect.width, rect.height));
    shape.setFillColor(toSFMLColor(fillColor));

    if (outlineThickness > 0.0f) {
        shape.setOutlineColor(toSFMLColor(outlineColor));
        shape.setOutlineThickness(outlineThickness);
    }

    window_->draw(shape);
}

void SFMLRenderer::display() {
    window_->display();
}

void SFMLRenderer::setCamera(const Camera& camera) {
    Vector2f pos = camera.getPosition();
    float zoom = camera.getZoom();
    IntRect viewport = camera.getViewport();

    sf::View view;
    view.setCenter(pos.x, pos.y);
    view.setSize(viewport.width / zoom, viewport.height / zoom);
    view.setViewport(sf::FloatRect(0.f, 0.f, 1.f, 1.f));

    window_->setView(view);
}

sf::Color SFMLRenderer::toSFMLColor(uint32_t rgba) {
    // Format: 0xRRGGBBAA
    uint8_t r = (rgba >> 24) & 0xFF;
    uint8_t g = (rgba >> 16) & 0xFF;
    uint8_t b = (rgba >> 8) & 0xFF;
    uint8_t a = rgba & 0xFF;
    return sf::Color(r, g, b, a);
}

}  // namespace sfml
}  // namespace rendering
}  // namespace engine
}  // namespace eng
