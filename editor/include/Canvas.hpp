#pragma once

#include "StageData.hpp"

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <string>
#include <unordered_map>

namespace Editor {

class Canvas {
public:
    void Init();
    void Render(EditorData& data, sf::RenderWindow& window);

private:
    sf::RenderTexture renderTexture_;
    bool initialized_ = false;
    float zoom_ = 1.0f;
    float panX_ = 0.0f;
    float panY_ = 0.0f;

    float timelineCursor_ = 0.0f;
    bool isPlaying_ = false;
    float playbackSpeed_ = 1.0f;

    int dragSpawnIndex_ = -1;
    bool isDragging_ = false;

    bool showTextures_ = false;
    bool texturesLoaded_ = false;
    std::unordered_map<std::string, sf::Texture> textureCache_;

    static constexpr float GameWidth = 1920.0f;
    static constexpr float GameHeight = 1080.0f;

    sf::Vector2f GameToCanvas(float gx, float gy, float canvasW, float canvasH) const;
    sf::Vector2f CanvasToGame(float cx, float cy, float canvasW, float canvasH) const;
    sf::Color GetEnemyColor(const std::string& category) const;

    void LoadTextures(const EditorData& data);
    void RenderCanvas(EditorData& data, float canvasW, float canvasH);
    void RenderTimeline(EditorData& data, float width);
    void HandleCanvasInteraction(EditorData& data, float canvasW, float canvasH);
};

}
