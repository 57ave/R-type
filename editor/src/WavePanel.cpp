#include "WavePanel.hpp"

#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <string>

namespace Editor {

void WavePanel::Render(EditorData& data) {
    if (data.levels.empty()) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "No levels loaded.");
        return;
    }

    auto& level = data.levels[data.selectedLevelIndex];

    if (ImGui::Button("+ Add Wave")) {
        WaveData newWave;
        if (!level.waves.empty()) {
            newWave.time = level.waves.back().time + 15.0f;
        } else {
            newWave.time = 3.0f;
        }
        WaveEnemy defaultEnemy;
        defaultEnemy.type = level.enemyTypes.empty() ? 0 : level.enemyTypes[0];
        defaultEnemy.count = 3;
        defaultEnemy.interval = 1.0f;
        newWave.enemies.push_back(defaultEnemy);
        level.waves.push_back(newWave);
        data.selectedWaveIndex = static_cast<int>(level.waves.size()) - 1;
        data.dirty = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Sort by Time")) {
        std::sort(level.waves.begin(), level.waves.end(),
                  [](const WaveData& a, const WaveData& b) { return a.time < b.time; });
        data.dirty = true;
    }

    ImGui::Separator();

    for (int i = 0; i < static_cast<int>(level.waves.size()); i++) {
        auto& wave = level.waves[i];
        ImGui::PushID(i);

        bool isSelected = (data.selectedWaveIndex == i);

        char label[256];
        int totalEnemies = 0;
        for (const auto& e : wave.enemies) totalEnemies += e.count;
        std::snprintf(label, sizeof(label), "Wave %d (t=%.1fs, %d enemies)",
                      i + 1, wave.time, totalEnemies);

        if (ImGui::Selectable(label, isSelected)) {
            data.selectedWaveIndex = i;
        }

        if (isSelected) {
            ImGui::Indent(16.0f);

            // Wave time
            ImGui::PushItemWidth(120);
            if (ImGui::DragFloat("Time##w", &wave.time, 0.5f, 0.0f, 600.0f, "%.1f")) {
                data.dirty = true;
            }
            ImGui::PopItemWidth();

            // Enemy entries table
            ImGui::Text("Enemies:");
            if (ImGui::BeginTable("EnemiesTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg,
                                  ImVec2(0, 0))) {
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed, 70.0f);
                ImGui::TableSetupColumn("Interval", ImGuiTableColumnFlags_WidthFixed, 90.0f);
                ImGui::TableSetupColumn("##del", ImGuiTableColumnFlags_WidthFixed, 30.0f);
                ImGui::TableHeadersRow();

                int toDelete = -1;
                for (int e = 0; e < static_cast<int>(wave.enemies.size()); e++) {
                    auto& enemy = wave.enemies[e];
                    ImGui::PushID(e);
                    ImGui::TableNextRow();

                    // Type dropdown
                    ImGui::TableSetColumnIndex(0);
                    ImGui::SetNextItemWidth(-1);
                    std::string typeLabel = (enemy.type >= 0 && enemy.type < EnemyTypeNameCount)
                                                ? EnemyTypeNames[enemy.type]
                                                : std::to_string(enemy.type);
                    if (ImGui::BeginCombo("##type", typeLabel.c_str())) {
                        for (int t = 0; t < EnemyTypeNameCount; t++) {
                            bool sel = (enemy.type == t);
                            if (ImGui::Selectable(EnemyTypeNames[t], sel)) {
                                enemy.type = t;
                                data.dirty = true;
                            }
                            if (sel) ImGui::SetItemDefaultFocus();
                        }
                        ImGui::EndCombo();
                    }

                    // Count
                    ImGui::TableSetColumnIndex(1);
                    ImGui::SetNextItemWidth(-1);
                    if (ImGui::InputInt("##count", &enemy.count, 0)) {
                        if (enemy.count < 1) enemy.count = 1;
                        data.dirty = true;
                    }

                    // Interval
                    ImGui::TableSetColumnIndex(2);
                    ImGui::SetNextItemWidth(-1);
                    if (ImGui::DragFloat("##interval", &enemy.interval, 0.05f, 0.1f, 10.0f, "%.2f")) {
                        data.dirty = true;
                    }

                    // Delete button
                    ImGui::TableSetColumnIndex(3);
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
                    if (ImGui::SmallButton("X")) {
                        toDelete = e;
                    }
                    ImGui::PopStyleColor();

                    ImGui::PopID();
                }

                ImGui::EndTable();

                if (toDelete >= 0) {
                    wave.enemies.erase(wave.enemies.begin() + toDelete);
                    data.dirty = true;
                }
            }

            // Add enemy entry button
            if (ImGui::SmallButton("+ Add Enemy Entry")) {
                WaveEnemy newEnemy;
                newEnemy.type = level.enemyTypes.empty() ? 0 : level.enemyTypes[0];
                newEnemy.count = 3;
                newEnemy.interval = 1.0f;
                wave.enemies.push_back(newEnemy);
                data.dirty = true;
            }

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
            if (ImGui::SmallButton("Delete Wave")) {
                showDeleteConfirm_ = true;
                deleteWaveIndex_ = i;
            }
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if (ImGui::SmallButton("Duplicate")) {
                WaveData copy = wave;
                copy.time = wave.time + 15.0f;
                level.waves.insert(level.waves.begin() + i + 1, copy);
                data.dirty = true;
            }

            ImGui::Unindent(16.0f);
        }

        ImGui::Separator();
        ImGui::PopID();
    }

    // Delete confirmation popup
    if (showDeleteConfirm_) {
        ImGui::OpenPopup("Delete Wave?");
        showDeleteConfirm_ = false;
    }
    if (ImGui::BeginPopupModal("Delete Wave?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this wave?");
        ImGui::Separator();

        if (ImGui::Button("Delete", ImVec2(120, 0))) {
            if (deleteWaveIndex_ >= 0 &&
                deleteWaveIndex_ < static_cast<int>(level.waves.size())) {
                level.waves.erase(level.waves.begin() + deleteWaveIndex_);
                if (data.selectedWaveIndex >= static_cast<int>(level.waves.size())) {
                    data.selectedWaveIndex =
                        std::max(0, static_cast<int>(level.waves.size()) - 1);
                }
                data.dirty = true;
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

}
