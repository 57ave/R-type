#pragma once

#include "StageData.hpp"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace Editor {

class Canvas {
public:
    void Init();
    void Render(EditorData& data, sf::RenderWindow& window);

private:
    sf::RenderTexture renderTexture_;
    bool initialized_ = false;

    float timelineCursor_ = 0.0f;
    bool isPlaying_ = false;
    float playbackSpeed_ = 1.0f;

    sf::Color GetEnemyTypeColor(int enemyType) const;

    void RenderCanvas(EditorData& data, float canvasW, float canvasH);
    void RenderTimeline(EditorData& data, float width);
};

}
