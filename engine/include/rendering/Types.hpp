#ifndef TYPES_HPP
#define TYPES_HPP

#include <cstdint>

namespace eng {
namespace engine {
namespace rendering {
struct Vector2u {
    uint32_t x;
    uint32_t y;

    Vector2u() : x(0), y(0) {}
    Vector2u(uint32_t x, uint32_t y) : x(x), y(y) {}
};

struct Vector2i {
    int x;
    int y;

    Vector2i() : x(0), y(0) {}
    Vector2i(int x, int y) : x(x), y(y) {}
};

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
    IntRect(int left, int top, int width, int height)
        : left(left), top(top), width(width), height(height) {}
};

struct FloatRect {
    float left;
    float top;
    float width;
    float height;

    FloatRect() : left(0.0f), top(0.0f), width(0.0f), height(0.0f) {}
    FloatRect(float left, float top, float width, float height)
        : left(left), top(top), width(width), height(height) {}

    bool contains(float x, float y) const {
        return x >= left && x <= left + width && y >= top && y <= top + height;
    }

    bool contains(const Vector2f& point) const { return contains(point.x, point.y); }
};

struct Transform {
    Vector2f position;
    float rotation;
    Vector2f scale;
    Transform() : position(0.0f, 0.0f), rotation(0.0f), scale(1.0f, 1.0f) {}
};
}  // namespace rendering
}  // namespace engine
}  // namespace eng

#endif