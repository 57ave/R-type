#include "Canvas.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace Editor {

void Canvas::Init() {
    if (!renderTexture_.create(960, 540)) {
        return;
    }
    initialized_ = true;
}

sf::Color Canvas::GetEnemyTypeColor(int enemyType) const {
    switch (enemyType) {
        case 0: return sf::Color(100, 220, 100);   // bug - green
        case 1: return sf::Color(100, 150, 255);   // fighter - blue
        case 2: return sf::Color(255, 100, 100);   // kamikaze - red
        case 3: return sf::Color(255, 200, 50);    // drone - yellow
        default: return sf::Color(180, 180, 180);
    }
}

void Canvas::Render(EditorData& data, sf::RenderWindow&) {
    if (!initialized_) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Canvas failed to initialize.");
        return;
    }

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float canvasW = avail.x;
    float canvasH = avail.y - 60.0f;
    if (canvasH < 100.0f) canvasH = 100.0f;

    sf::Vector2u rtSize = renderTexture_.getSize();
    if (static_cast<unsigned>(canvasW) != rtSize.x ||
        static_cast<unsigned>(canvasH) != rtSize.y) {
        if (canvasW > 10 && canvasH > 10) {
            renderTexture_.create(static_cast<unsigned>(canvasW),
                                 static_cast<unsigned>(canvasH));
        }
    }

    RenderCanvas(data, canvasW, canvasH);

    ImGui::Image(renderTexture_.getTexture(), sf::Vector2f(canvasW, canvasH));

    ImGui::Separator();
    RenderTimeline(data, canvasW);
}

void Canvas::RenderCanvas(EditorData& data, float canvasW, float canvasH) {
    renderTexture_.clear(sf::Color(25, 25, 35));

    if (data.levels.empty()) {
        renderTexture_.display();
        return;
    }

    auto& level = data.levels[data.selectedLevelIndex];
    if (level.waves.empty()) {
        renderTexture_.display();
        return;
    }

    // Draw border
    sf::RectangleShape border(sf::Vector2f(canvasW - 4.0f, canvasH - 4.0f));
    border.setPosition(2.0f, 2.0f);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(60, 60, 80));
    border.setOutlineThickness(1.0f);
    renderTexture_.draw(border);

    // Calculate time range
    float maxTime = level.boss.spawnTime + 10.0f;
    for (const auto& wave : level.waves) {
        float waveEnd = wave.time;
        for (const auto& e : wave.enemies) {
            waveEnd = std::max(waveEnd, wave.time + e.count * e.interval);
        }
        maxTime = std::max(maxTime, waveEnd + 5.0f);
    }

    float margin = 40.0f;
    float barAreaW = canvasW - margin * 2.0f;
    float barAreaH = canvasH - margin * 2.0f;

    // Draw time grid lines
    float timeStep = 10.0f;
    if (maxTime > 200.0f) timeStep = 30.0f;
    else if (maxTime > 100.0f) timeStep = 20.0f;

    for (float t = 0.0f; t <= maxTime; t += timeStep) {
        float x = margin + (t / maxTime) * barAreaW;
        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(x, margin), sf::Color(40, 40, 55)),
            sf::Vertex(sf::Vector2f(x, canvasH - margin), sf::Color(40, 40, 55)),
        };
        renderTexture_.draw(line, 2, sf::Lines);
    }

    // Draw each wave as colored bars
    float waveHeight = std::min(40.0f, barAreaH / static_cast<float>(level.waves.size()) - 4.0f);
    if (waveHeight < 10.0f) waveHeight = 10.0f;

    for (int w = 0; w < static_cast<int>(level.waves.size()); w++) {
        auto& wave = level.waves[w];
        bool isSelected = (data.selectedWaveIndex == w);

        float yBase = margin + w * (waveHeight + 4.0f);
        if (yBase + waveHeight > canvasH - margin) break;

        // Draw enemy bars stacked within the wave
        float enemyBarH = waveHeight / static_cast<float>(std::max(1, static_cast<int>(wave.enemies.size())));

        for (int e = 0; e < static_cast<int>(wave.enemies.size()); e++) {
            auto& enemy = wave.enemies[e];
            float startX = margin + (wave.time / maxTime) * barAreaW;
            float duration = enemy.count * enemy.interval;
            float barW = (duration / maxTime) * barAreaW;
            if (barW < 4.0f) barW = 4.0f;

            sf::RectangleShape bar(sf::Vector2f(barW, enemyBarH - 1.0f));
            bar.setPosition(startX, yBase + e * enemyBarH);

            sf::Color color = GetEnemyTypeColor(enemy.type);
            if (!isSelected) {
                color.a = 150;
            }
            bar.setFillColor(color);

            if (isSelected) {
                bar.setOutlineColor(sf::Color::White);
                bar.setOutlineThickness(1.0f);
            }

            renderTexture_.draw(bar);
        }

        // Wave label indicator
        float labelX = margin + (wave.time / maxTime) * barAreaW - 2.0f;
        sf::RectangleShape marker(sf::Vector2f(2.0f, waveHeight));
        marker.setPosition(labelX, yBase);
        marker.setFillColor(isSelected ? sf::Color::White : sf::Color(150, 150, 150));
        renderTexture_.draw(marker);
    }

    // Boss spawn marker
    {
        float bossX = margin + (level.boss.spawnTime / maxTime) * barAreaW;
        sf::Vertex bossLine[] = {
            sf::Vertex(sf::Vector2f(bossX, margin), sf::Color(255, 60, 60, 200)),
            sf::Vertex(sf::Vector2f(bossX, canvasH - margin), sf::Color(255, 60, 60, 200)),
        };
        renderTexture_.draw(bossLine, 2, sf::Lines);

        // Boss marker diamond
        sf::CircleShape diamond(8.0f, 4);
        diamond.setOrigin(8.0f, 8.0f);
        diamond.setPosition(bossX, margin - 12.0f);
        diamond.setFillColor(sf::Color(255, 60, 60));
        renderTexture_.draw(diamond);
    }

    // Timeline cursor
    if (timelineCursor_ >= 0.0f && timelineCursor_ <= maxTime) {
        float cursorX = margin + (timelineCursor_ / maxTime) * barAreaW;
        sf::Vertex cursorLine[] = {
            sf::Vertex(sf::Vector2f(cursorX, 0.0f), sf::Color(255, 255, 100, 180)),
            sf::Vertex(sf::Vector2f(cursorX, canvasH), sf::Color(255, 255, 100, 180)),
        };
        renderTexture_.draw(cursorLine, 2, sf::Lines);
    }

    renderTexture_.display();
}

void Canvas::RenderTimeline(EditorData& data, float width) {
    if (data.levels.empty()) return;
    auto& level = data.levels[data.selectedLevelIndex];

    float maxTime = level.boss.spawnTime + 10.0f;
    for (const auto& wave : level.waves) {
        for (const auto& e : wave.enemies) {
            maxTime = std::max(maxTime, wave.time + e.count * e.interval + 5.0f);
        }
    }

    if (ImGui::Button(isPlaying_ ? "Pause" : "Play")) {
        isPlaying_ = !isPlaying_;
    }
    ImGui::SameLine();

    ImGui::PushItemWidth(60);
    ImGui::DragFloat("##speed", &playbackSpeed_, 0.1f, 0.1f, 5.0f, "%.1fx");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    if (isPlaying_) {
        timelineCursor_ += ImGui::GetIO().DeltaTime * playbackSpeed_;
        if (timelineCursor_ > maxTime) {
            timelineCursor_ = 0.0f;
        }
    }

    timelineCursor_ = std::clamp(timelineCursor_, 0.0f, maxTime);

    ImGui::PushItemWidth(width - 200.0f);
    ImGui::SliderFloat("##timeline", &timelineCursor_, 0.0f, maxTime, "%.1fs");
    ImGui::PopItemWidth();

    // Draw wave markers on the timeline slider
    ImVec2 sliderMin = ImGui::GetItemRectMin();
    ImVec2 sliderMax = ImGui::GetItemRectMax();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (int w = 0; w < static_cast<int>(level.waves.size()); w++) {
        auto& wave = level.waves[w];
        float t = (maxTime > 0.0f) ? (wave.time / maxTime) : 0.0f;
        t = std::clamp(t, 0.0f, 1.0f);
        float x = sliderMin.x + t * (sliderMax.x - sliderMin.x);

        bool isSelected = (data.selectedWaveIndex == w);
        float markerRadius = isSelected ? 5.0f : 3.0f;

        // Color based on most common enemy in wave
        int mainType = wave.enemies.empty() ? 0 : wave.enemies[0].type;
        sf::Color sfColor = GetEnemyTypeColor(mainType);
        ImU32 color = IM_COL32(sfColor.r, sfColor.g, sfColor.b, 200);

        drawList->AddCircleFilled(ImVec2(x, sliderMin.y - 4.0f), markerRadius, color);
    }

    // Boss marker on timeline
    {
        float t = (maxTime > 0.0f) ? (level.boss.spawnTime / maxTime) : 0.0f;
        t = std::clamp(t, 0.0f, 1.0f);
        float x = sliderMin.x + t * (sliderMax.x - sliderMin.x);
        drawList->AddTriangleFilled(
            ImVec2(x - 5.0f, sliderMin.y - 10.0f),
            ImVec2(x + 5.0f, sliderMin.y - 10.0f),
            ImVec2(x, sliderMin.y - 3.0f),
            IM_COL32(255, 60, 60, 220));
    }
}

}
