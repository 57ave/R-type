#include "Canvas.hpp"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/Window/Mouse.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <iostream>

namespace Editor {

void Canvas::Init() {
    if (!renderTexture_.create(960, 540)) {
        return;
    }
    initialized_ = true;
}

void Canvas::LoadTextures(const EditorData& data) {
    if (data.assetsBasePath.empty()) {
        std::cerr << "[Canvas] No assets base path set, cannot load textures" << std::endl;
        texturesLoaded_ = true;
        return;
    }

    namespace fs = std::filesystem;

    for (const auto& enemy : data.enemyTypes) {
        if (enemy.texture.empty()) continue;
        if (textureCache_.count(enemy.texture)) continue;

        fs::path texPath = fs::path(data.assetsBasePath) / enemy.texture;
        if (!fs::exists(texPath)) {
            std::cerr << "[Canvas] Texture not found: " << texPath << std::endl;
            continue;
        }

        sf::Texture tex;
        if (tex.loadFromFile(texPath.string())) {
            tex.setSmooth(true);
            textureCache_[enemy.texture] = std::move(tex);
            std::cout << "[Canvas] Loaded texture: " << texPath << std::endl;
        } else {
            std::cerr << "[Canvas] Failed to load texture: " << texPath << std::endl;
        }
    }

    texturesLoaded_ = true;
}

sf::Vector2f Canvas::GameToCanvas(float gx, float gy, float canvasW, float canvasH) const {
    float scaleX = canvasW / GameWidth * zoom_;
    float scaleY = canvasH / GameHeight * zoom_;
    return sf::Vector2f((gx * scaleX) + panX_, canvasH - ((gy * scaleY) + panY_));
}

sf::Vector2f Canvas::CanvasToGame(float cx, float cy, float canvasW, float canvasH) const {
    float scaleX = canvasW / GameWidth * zoom_;
    float scaleY = canvasH / GameHeight * zoom_;
    return sf::Vector2f((cx - panX_) / scaleX, (cy - panY_) / scaleY);
}

sf::Color Canvas::GetEnemyColor(const std::string& category) const {
    if (category == "common") return sf::Color(100, 220, 100);
    if (category == "medium") return sf::Color(220, 200, 60);
    if (category == "elite") return sf::Color(230, 130, 50);
    if (category == "special") return sf::Color(80, 200, 220);
    return sf::Color(180, 180, 180);
}

void Canvas::Render(EditorData& data, sf::RenderWindow&) {
    if (!initialized_) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Canvas failed to initialize.");
        return;
    }

    ImGui::Checkbox("Show Textures", &showTextures_);
    if (showTextures_ && !texturesLoaded_) {
        LoadTextures(data);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset View")) {
        zoom_ = 1.0f;
        panX_ = 0.0f;
        panY_ = 0.0f;
    }
    ImGui::SameLine();
    ImGui::TextDisabled("| Scroll=Zoom  MMB=Pan  DblClick=Add  Drag=Move Y");

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

    HandleCanvasInteraction(data, canvasW, canvasH);

    ImGui::Separator();
    RenderTimeline(data, canvasW);
}

void Canvas::RenderCanvas(EditorData& data, float canvasW, float canvasH) {
    renderTexture_.clear(sf::Color(25, 25, 35));

    if (data.stages.empty()) return;
    auto& stage = data.stages[data.selectedStageIndex];
    if (data.selectedWaveIndex < 0 ||
        data.selectedWaveIndex >= static_cast<int>(stage.waves.size())) {
        renderTexture_.display();
        return;
    }

    sf::RectangleShape border(sf::Vector2f(canvasW - 4.0f, canvasH - 4.0f));
    border.setPosition(2.0f, 2.0f);
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color(60, 60, 80));
    border.setOutlineThickness(1.0f);
    renderTexture_.draw(border);

    float scaleY = canvasH / GameHeight * zoom_;
    for (float gy = 0.0f; gy <= GameHeight; gy += 100.0f) {
        float cy = canvasH - (gy * scaleY + panY_);
        if (cy < 0 || cy > canvasH) continue;

        sf::Vertex line[] = {
            sf::Vertex(sf::Vector2f(0.0f, cy), sf::Color(40, 40, 55)),
            sf::Vertex(sf::Vector2f(canvasW, cy), sf::Color(40, 40, 55)),
        };
        renderTexture_.draw(line, 2, sf::Lines);
    }

    float scaleX = canvasW / GameWidth * zoom_;
    float spawnLineX = GameWidth * scaleX + panX_;
    if (spawnLineX < canvasW) {
        sf::Vertex spawnLine[] = {
            sf::Vertex(sf::Vector2f(spawnLineX, 0.0f), sf::Color(80, 40, 40)),
            sf::Vertex(sf::Vector2f(spawnLineX, canvasH), sf::Color(80, 40, 40)),
        };
        renderTexture_.draw(spawnLine, 2, sf::Lines);
    }

    auto& wave = stage.waves[data.selectedWaveIndex];
    for (int s = 0; s < static_cast<int>(wave.spawns.size()); s++) {
        auto& spawn = wave.spawns[s];

        const EnemyTypeInfo* enemyInfo = nullptr;
        std::string category = "common";
        for (const auto& et : data.enemyTypes) {
            if (et.key == spawn.enemy) {
                enemyInfo = &et;
                category = et.category;
                break;
            }
        }

        sf::Color color = GetEnemyColor(category);
        bool isSelected = (data.selectedSpawnIndex == s);

        float normalizedTime = (wave.duration > 0.0f) ? (spawn.time / wave.duration) : 0.0f;
        float gx = GameWidth * (0.6f + normalizedTime * 0.35f);
        float gy = spawn.y;

        sf::Vector2f pos = GameToCanvas(gx, gy, canvasW, canvasH);

        if (isSelected) {
            float ringRadius = 14.0f;
            sf::CircleShape ring(ringRadius);
            ring.setOrigin(ringRadius, ringRadius);
            ring.setPosition(pos);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineColor(sf::Color::White);
            ring.setOutlineThickness(2.0f);
            renderTexture_.draw(ring);
        }

        bool textureRendered = false;
        if (showTextures_ && enemyInfo && !enemyInfo->texture.empty()) {
            auto it = textureCache_.find(enemyInfo->texture);
            if (it != textureCache_.end()) {
                sf::Sprite sprite(it->second);

                int startX = 0;
                int startY = 0;
                sprite.setTextureRect(sf::IntRect(startX, startY,
                                                  enemyInfo->frameWidth,
                                                  enemyInfo->frameHeight));

                float displayScale = enemyInfo->scale * 0.6f * zoom_;
                sprite.setScale(displayScale, displayScale);

                sprite.setOrigin(static_cast<float>(enemyInfo->frameWidth) / 2.0f,
                                 static_cast<float>(enemyInfo->frameHeight) / 2.0f);
                sprite.setPosition(pos);

                if (isSelected) {
                    sprite.setColor(sf::Color(255, 255, 255, 255));
                } else {
                    sprite.setColor(sf::Color(255, 255, 255, 220));
                }

                renderTexture_.draw(sprite);
                textureRendered = true;

                if (spawn.count > 1) {
                    for (int c = 1; c < std::min(spawn.count, 4); c++) {
                        sf::Sprite extra(it->second);
                        extra.setTextureRect(sf::IntRect(startX, startY,
                                                         enemyInfo->frameWidth,
                                                         enemyInfo->frameHeight));
                        float extraScale = displayScale * 0.6f;
                        extra.setScale(extraScale, extraScale);
                        extra.setOrigin(static_cast<float>(enemyInfo->frameWidth) / 2.0f,
                                        static_cast<float>(enemyInfo->frameHeight) / 2.0f);
                        extra.setPosition(pos.x + c * 12.0f * zoom_, pos.y - c * 12.0f * zoom_);
                        extra.setColor(sf::Color(255, 255, 255, 120));
                        renderTexture_.draw(extra);
                    }
                }
            }
        }

        if (!textureRendered) {
            float radius = isSelected ? 10.0f : 7.0f;

            sf::CircleShape dot(radius);
            dot.setOrigin(radius, radius);
            dot.setPosition(pos);
            dot.setFillColor(color);
            if (isSelected) {
                dot.setOutlineColor(sf::Color::White);
                dot.setOutlineThickness(1.0f);
            }
            renderTexture_.draw(dot);

            if (spawn.count > 1) {
                for (int c = 1; c < std::min(spawn.count, 5); c++) {
                    sf::CircleShape extra(5.0f);
                    extra.setOrigin(5.0f, 5.0f);
                    extra.setPosition(pos.x + c * 6.0f, pos.y - c * 6.0f);
                    sf::Color faded = color;
                    faded.a = 120;
                    extra.setFillColor(faded);
                    renderTexture_.draw(extra);
                }
            }
        }

        if (textureRendered) {
            sf::CircleShape indicator(3.0f);
            indicator.setOrigin(3.0f, 3.0f);
            indicator.setPosition(pos.x, pos.y + 20.0f * zoom_);
            indicator.setFillColor(color);
            renderTexture_.draw(indicator);
        }
    }

    if (wave.duration > 0.0f) {
        float cursorNorm = (timelineCursor_ - wave.startTime) / wave.duration;
        cursorNorm = std::clamp(cursorNorm, 0.0f, 1.0f);
        float cursorGx = GameWidth * (0.6f + cursorNorm * 0.35f);
        float cursorCx = cursorGx * scaleX + panX_;

        sf::Vertex cursorLine[] = {
            sf::Vertex(sf::Vector2f(cursorCx, 0.0f), sf::Color(255, 100, 100, 150)),
            sf::Vertex(sf::Vector2f(cursorCx, canvasH), sf::Color(255, 100, 100, 150)),
        };
        renderTexture_.draw(cursorLine, 2, sf::Lines);
    }

    renderTexture_.display();
}

void Canvas::HandleCanvasInteraction(EditorData& data, float canvasW, float canvasH) {
    if (data.stages.empty()) return;
    auto& stage = data.stages[data.selectedStageIndex];
    if (data.selectedWaveIndex < 0 ||
        data.selectedWaveIndex >= static_cast<int>(stage.waves.size()))
        return;

    auto& wave = stage.waves[data.selectedWaveIndex];

    if (!ImGui::IsItemHovered()) {
        isDragging_ = false;
        return;
    }

    float wheel = ImGui::GetIO().MouseWheel;
    if (std::abs(wheel) > 0.01f) {
        zoom_ = std::clamp(zoom_ + wheel * 0.1f, 0.3f, 3.0f);
    }

    if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
        ImVec2 delta = ImGui::GetIO().MouseDelta;
        panX_ += delta.x;
        panY_ += delta.y;
    }

    ImVec2 mousePos = ImGui::GetMousePos();
    ImVec2 itemMin = ImGui::GetItemRectMin();
    float localX = mousePos.x - itemMin.x;
    float localY = mousePos.y - itemMin.y;

    sf::Vector2f gamePos = CanvasToGame(localX, localY, canvasW, canvasH);

    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        int closest = -1;
        float closestDist = showTextures_ ? 30.0f : 20.0f;

        for (int s = 0; s < static_cast<int>(wave.spawns.size()); s++) {
            auto& spawn = wave.spawns[s];
            float normalizedTime =
                (wave.duration > 0.0f) ? (spawn.time / wave.duration) : 0.0f;
            float gx = GameWidth * (0.6f + normalizedTime * 0.35f);
            float gy = spawn.y;

            sf::Vector2f cpos = GameToCanvas(gx, gy, canvasW, canvasH);
            float dx = localX - cpos.x;
            float dy = localY - cpos.y;
            float dist = std::sqrt(dx * dx + dy * dy);
            if (dist < closestDist) {
                closestDist = dist;
                closest = s;
            }
        }

        if (closest >= 0) {
            data.selectedSpawnIndex = closest;
            isDragging_ = true;
            dragSpawnIndex_ = closest;
        } else {
            data.selectedSpawnIndex = -1;
            isDragging_ = false;
        }
    }

    if (isDragging_ && ImGui::IsMouseDragging(ImGuiMouseButton_Left) && dragSpawnIndex_ >= 0 &&
        dragSpawnIndex_ < static_cast<int>(wave.spawns.size())) {
        float newY = std::clamp(gamePos.y, 0.0f, GameHeight);
        wave.spawns[dragSpawnIndex_].y = newY;
        data.dirty = true;
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
        isDragging_ = false;
    }

    if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && data.selectedSpawnIndex < 0) {
        SpawnData newSpawn;
        float normalizedX = (gamePos.x / GameWidth - 0.6f) / 0.35f;
        normalizedX = std::clamp(normalizedX, 0.0f, 1.0f);
        newSpawn.time = normalizedX * wave.duration;
        newSpawn.y = std::clamp(gamePos.y, 0.0f, GameHeight);
        newSpawn.enemy = "basic";
        newSpawn.pattern = "straight";
        newSpawn.editorId = data.nextSpawnId++;
        wave.spawns.push_back(newSpawn);
        data.selectedSpawnIndex = static_cast<int>(wave.spawns.size()) - 1;
        data.dirty = true;
    }

    if (data.selectedSpawnIndex >= 0 &&
        data.selectedSpawnIndex < static_cast<int>(wave.spawns.size())) {
        auto& spawn = wave.spawns[data.selectedSpawnIndex];
        ImGui::SetTooltip("%.1fs  %s  Y=%.0f  %s", spawn.time, spawn.enemy.c_str(), spawn.y,
                          spawn.pattern.c_str());
    }
}

void Canvas::RenderTimeline(EditorData& data, float width) {
    if (data.stages.empty()) return;
    auto& stage = data.stages[data.selectedStageIndex];
    if (data.selectedWaveIndex < 0 ||
        data.selectedWaveIndex >= static_cast<int>(stage.waves.size()))
        return;

    auto& wave = stage.waves[data.selectedWaveIndex];

    if (ImGui::Button(isPlaying_ ? "Pause" : "Play")) {
        isPlaying_ = !isPlaying_;
    }
    ImGui::SameLine();

    ImGui::PushItemWidth(60);
    ImGui::DragFloat("##speed", &playbackSpeed_, 0.1f, 0.1f, 5.0f, "%.1fx");
    ImGui::PopItemWidth();
    ImGui::SameLine();

    float minTime = wave.startTime;
    float maxTime = wave.startTime + wave.duration;

    if (isPlaying_) {
        timelineCursor_ += ImGui::GetIO().DeltaTime * playbackSpeed_;
        if (timelineCursor_ > maxTime) {
            timelineCursor_ = minTime;
        }
    }

    timelineCursor_ = std::clamp(timelineCursor_, minTime, maxTime);

    ImGui::PushItemWidth(width - 200.0f);
    ImGui::SliderFloat("##timeline", &timelineCursor_, minTime, maxTime, "%.1fs");
    ImGui::PopItemWidth();

    ImVec2 sliderMin = ImGui::GetItemRectMin();
    ImVec2 sliderMax = ImGui::GetItemRectMax();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    for (int s = 0; s < static_cast<int>(wave.spawns.size()); s++) {
        auto& spawn = wave.spawns[s];
        float t = (wave.duration > 0.0f) ? ((spawn.time) / wave.duration) : 0.0f;
        t = std::clamp(t, 0.0f, 1.0f);
        float x = sliderMin.x + t * (sliderMax.x - sliderMin.x);

        std::string category = "common";
        for (const auto& et : data.enemyTypes) {
            if (et.key == spawn.enemy) {
                category = et.category;
                break;
            }
        }

        sf::Color sfColor = GetEnemyColor(category);
        ImU32 color = IM_COL32(sfColor.r, sfColor.g, sfColor.b, 200);

        bool isSelected = (data.selectedSpawnIndex == s);
        float markerRadius = isSelected ? 5.0f : 3.0f;

        drawList->AddCircleFilled(ImVec2(x, sliderMin.y - 4.0f), markerRadius, color);
    }
}

}
