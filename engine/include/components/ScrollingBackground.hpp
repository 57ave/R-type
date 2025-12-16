#ifndef RTYPE_ENGINE_COMPONENTS_SCROLLINGBACKGROUND_HPP
#define RTYPE_ENGINE_COMPONENTS_SCROLLINGBACKGROUND_HPP

struct ScrollingBackground {
    float scrollSpeed = 200.0f;
    bool horizontal = true;        // Horizontal or vertical scrolling
    bool loop = true;              // Loop infinitely

    // For multi-sprite parallax
    float sprite1X = 0.0f;
    float sprite2X = 0.0f;
    float spriteWidth = 0.0f;      // Width of one sprite
};

struct BackgroundTag {
    int layer = 0;                 // Render layer (-10 = far background)
};

#endif // RTYPE_ENGINE_COMPONENTS_SCROLLINGBACKGROUND_HPP
