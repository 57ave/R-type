#include "WavePanel.hpp"

#include <imgui.h>

#include <cstdio>
#include <string>

namespace Editor {

void WavePanel::Render(EditorData& data) {
    if (data.stages.empty()) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "No stages loaded.");
        return;
    }

    auto& stage = data.stages[data.selectedStageIndex];

    if (ImGui::Button("+ Add Wave")) {
        WaveData newWave;
        newWave.name = "New Wave";
        if (!stage.waves.empty()) {
            auto& last = stage.waves.back();
            newWave.startTime = last.startTime + last.duration + 3.0f;
        }
        newWave.duration = 30.0f;
        stage.waves.push_back(newWave);
        data.selectedWaveIndex = static_cast<int>(stage.waves.size()) - 1;
        data.selectedSpawnIndex = -1;
        data.dirty = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("+ Add Boss Wave")) {
        WaveData bossWave;
        bossWave.name = "BOSS";
        bossWave.isBossWave = true;
        bossWave.boss = "stage" + std::to_string(stage.stageNumber) + "_boss";
        if (!stage.waves.empty()) {
            auto& last = stage.waves.back();
            bossWave.startTime = last.startTime + last.duration + 3.0f;
        }
        bossWave.duration = 60.0f;
        stage.waves.push_back(bossWave);
        data.selectedWaveIndex = static_cast<int>(stage.waves.size()) - 1;
        data.selectedSpawnIndex = -1;
        data.dirty = true;
    }

    ImGui::Separator();

    for (int i = 0; i < static_cast<int>(stage.waves.size()); i++) {
        auto& wave = stage.waves[i];
        ImGui::PushID(i);

        bool isSelected = (data.selectedWaveIndex == i);

        if (wave.isBossWave) {
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.2f, 0.2f, 0.6f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.7f, 0.3f, 0.3f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.8f, 0.3f, 0.3f, 0.8f));
        }

        char label[256];
        if (wave.isBossWave) {
            std::snprintf(label, sizeof(label), "[BOSS] %s", wave.name.c_str());
        } else {
            std::snprintf(label, sizeof(label), "Wave %d: %s", i + 1, wave.name.c_str());
        }

        if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
            data.selectedWaveIndex = i;
            data.selectedSpawnIndex = -1;
        }

        if (wave.isBossWave) {
            ImGui::PopStyleColor(3);
        }

        ImGui::Indent(16.0f);
        ImGui::TextDisabled("Start: %.1fs  Duration: %.1fs  Spawns: %d", wave.startTime,
                            wave.duration, static_cast<int>(wave.spawns.size()));

        if (isSelected) {
            ImGui::PushItemWidth(100);
            if (ImGui::DragFloat("Start##w", &wave.startTime, 0.5f, 0.0f, 600.0f, "%.1f")) {
                data.dirty = true;
            }
            ImGui::SameLine();
            if (ImGui::DragFloat("Dur##w", &wave.duration, 0.5f, 1.0f, 300.0f, "%.1f")) {
                data.dirty = true;
            }
            ImGui::PopItemWidth();

            char nameBuf[128];
            std::snprintf(nameBuf, sizeof(nameBuf), "%s", wave.name.c_str());
            ImGui::PushItemWidth(200);
            if (ImGui::InputText("Name##w", nameBuf, sizeof(nameBuf))) {
                wave.name = nameBuf;
                data.dirty = true;
            }
            ImGui::PopItemWidth();

            if (wave.isBossWave) {
                char bossBuf[64];
                std::snprintf(bossBuf, sizeof(bossBuf), "%s", wave.boss.c_str());
                ImGui::PushItemWidth(200);
                if (ImGui::InputText("Boss Type##w", bossBuf, sizeof(bossBuf))) {
                    wave.boss = bossBuf;
                    data.dirty = true;
                }
                ImGui::PopItemWidth();
            }

            bool hasReward = wave.reward.has_value();
            if (ImGui::Checkbox("Has Reward", &hasReward)) {
                if (hasReward && !wave.reward) {
                    wave.reward = RewardData{"weapon_upgrade", 400.0f};
                } else if (!hasReward) {
                    wave.reward.reset();
                }
                data.dirty = true;
            }
            if (wave.reward) {
                char rewardBuf[64];
                std::snprintf(rewardBuf, sizeof(rewardBuf), "%s", wave.reward->type.c_str());
                ImGui::PushItemWidth(150);
                if (ImGui::InputText("Reward##w", rewardBuf, sizeof(rewardBuf))) {
                    wave.reward->type = rewardBuf;
                    data.dirty = true;
                }
                ImGui::SameLine();
                if (ImGui::DragFloat("Y##reward", &wave.reward->y, 1.0f, 0.0f, 1080.0f,
                                     "%.0f")) {
                    data.dirty = true;
                }
                ImGui::PopItemWidth();
            }

            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
            if (ImGui::Button("Delete Wave")) {
                showDeleteConfirm_ = true;
                deleteWaveIndex_ = i;
            }
            ImGui::PopStyleColor();

            ImGui::SameLine();
            if (ImGui::Button("Duplicate Wave")) {
                WaveData copy = wave;
                copy.name = wave.name + " (copy)";
                copy.startTime = wave.startTime + wave.duration + 3.0f;
                stage.waves.insert(stage.waves.begin() + i + 1, copy);
                data.dirty = true;
            }
        }

        ImGui::Unindent(16.0f);
        ImGui::Separator();
        ImGui::PopID();
    }

    if (showDeleteConfirm_) {
        ImGui::OpenPopup("Delete Wave?");
        showDeleteConfirm_ = false;
    }
    if (ImGui::BeginPopupModal("Delete Wave?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to delete this wave?");
        ImGui::Separator();

        if (ImGui::Button("Delete", ImVec2(120, 0))) {
            if (deleteWaveIndex_ >= 0 &&
                deleteWaveIndex_ < static_cast<int>(stage.waves.size())) {
                stage.waves.erase(stage.waves.begin() + deleteWaveIndex_);
                if (data.selectedWaveIndex >= static_cast<int>(stage.waves.size())) {
                    data.selectedWaveIndex =
                        std::max(0, static_cast<int>(stage.waves.size()) - 1);
                }
                data.selectedSpawnIndex = -1;
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
