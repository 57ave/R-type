#include "SpawnTable.hpp"

#include <imgui.h>

#include <algorithm>
#include <cstdio>
#include <string>

namespace Editor {

void SpawnTable::Render(EditorData& data) {
    if (data.stages.empty()) return;

    auto& stage = data.stages[data.selectedStageIndex];
    if (data.selectedWaveIndex < 0 ||
        data.selectedWaveIndex >= static_cast<int>(stage.waves.size())) {
        ImGui::TextColored(ImVec4(1, 0.8f, 0.4f, 1), "Select a wave to edit spawns.");
        return;
    }

    auto& wave = stage.waves[data.selectedWaveIndex];

    ImGui::Text("Wave: %s  |  Spawns: %d", wave.name.c_str(),
                static_cast<int>(wave.spawns.size()));
    ImGui::Separator();

    if (ImGui::Button("+ Add Spawn")) {
        SpawnData newSpawn;
        newSpawn.time = 0.0f;
        if (!wave.spawns.empty()) {
            newSpawn.time = wave.spawns.back().time + 1.0f;
        }
        newSpawn.enemy = "basic";
        newSpawn.y = 400.0f;
        newSpawn.pattern = "straight";
        newSpawn.editorId = data.nextSpawnId++;
        wave.spawns.push_back(newSpawn);
        data.dirty = true;
    }

    ImGui::SameLine();
    if (ImGui::Button("Sort by Time")) {
        std::sort(wave.spawns.begin(), wave.spawns.end(),
                  [](const SpawnData& a, const SpawnData& b) { return a.time < b.time; });
    }

    if (data.selectedSpawnIndex >= 0 &&
        data.selectedSpawnIndex < static_cast<int>(wave.spawns.size())) {
        ImGui::SameLine();
        if (ImGui::Button("Duplicate Selected")) {
            SpawnData copy = wave.spawns[data.selectedSpawnIndex];
            copy.time += 0.5f;
            copy.editorId = data.nextSpawnId++;
            copy.selected = false;
            wave.spawns.insert(wave.spawns.begin() + data.selectedSpawnIndex + 1, copy);
            data.selectedSpawnIndex++;
            data.dirty = true;
        }

        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.2f, 0.2f, 1.0f));
        if (ImGui::Button("Delete Selected")) {
            wave.spawns.erase(wave.spawns.begin() + data.selectedSpawnIndex);
            data.selectedSpawnIndex = -1;
            data.dirty = true;
            ImGui::PopStyleColor();
            return;
        }
        ImGui::PopStyleColor();
    }

    ImGui::Separator();

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
                                 ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY |
                                 ImGuiTableFlags_SizingStretchProp;

    if (ImGui::BeginTable("SpawnsTable", 7, tableFlags,
                          ImVec2(0.0f, ImGui::GetContentRegionAvail().y))) {
        ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Enemy", ImGuiTableColumnFlags_WidthFixed, 130.0f);
        ImGui::TableSetupColumn("Y Pos", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Pattern", ImGuiTableColumnFlags_WidthFixed, 110.0f);
        ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Spacing", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("##del", ImGuiTableColumnFlags_WidthFixed, 30.0f);
        ImGui::TableHeadersRow();

        int toDelete = -1;

        for (int s = 0; s < static_cast<int>(wave.spawns.size()); s++) {
            auto& spawn = wave.spawns[s];
            ImGui::PushID(spawn.editorId);
            ImGui::TableNextRow();

            bool isSelected = (data.selectedSpawnIndex == s);

            ImGui::TableSetColumnIndex(0);
            if (isSelected) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1,
                                       ImGui::GetColorU32(ImVec4(0.2f, 0.3f, 0.6f, 0.4f)));
            }
            ImGui::SetNextItemWidth(-1);
            if (ImGui::DragFloat("##time", &spawn.time, 0.1f, 0.0f, wave.duration, "%.1f")) {
                data.dirty = true;
            }
            if (ImGui::IsItemClicked()) {
                data.selectedSpawnIndex = s;
            }

            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-1);
            if (ImGui::BeginCombo("##enemy", spawn.enemy.c_str())) {
                for (const auto& enemyType : data.enemyTypes) {
                    bool sel = (spawn.enemy == enemyType.key);
                    std::string label = enemyType.key + " (" + enemyType.name + ")";
                    if (ImGui::Selectable(label.c_str(), sel)) {
                        spawn.enemy = enemyType.key;
                        data.dirty = true;
                    }
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (ImGui::IsItemClicked()) {
                data.selectedSpawnIndex = s;
            }

            ImGui::TableSetColumnIndex(2);
            ImGui::SetNextItemWidth(-1);
            if (ImGui::DragFloat("##y", &spawn.y, 1.0f, 0.0f, 1080.0f, "%.0f")) {
                data.dirty = true;
            }
            if (ImGui::IsItemClicked()) {
                data.selectedSpawnIndex = s;
            }

            ImGui::TableSetColumnIndex(3);
            ImGui::SetNextItemWidth(-1);
            if (ImGui::BeginCombo("##pattern", spawn.pattern.c_str())) {
                for (int p = 0; p < KnownPatternCount; p++) {
                    bool sel = (spawn.pattern == KnownPatterns[p]);
                    if (ImGui::Selectable(KnownPatterns[p], sel)) {
                        spawn.pattern = KnownPatterns[p];
                        data.dirty = true;
                    }
                    if (sel) ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            if (ImGui::IsItemClicked()) {
                data.selectedSpawnIndex = s;
            }

            ImGui::TableSetColumnIndex(4);
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputInt("##count", &spawn.count, 0)) {
                if (spawn.count < 1) spawn.count = 1;
                data.dirty = true;
            }
            if (ImGui::IsItemClicked()) {
                data.selectedSpawnIndex = s;
            }

            ImGui::TableSetColumnIndex(5);
            ImGui::SetNextItemWidth(-1);
            if (spawn.count > 1) {
                if (ImGui::DragFloat("##spacing", &spawn.spacing, 0.05f, 0.05f, 5.0f, "%.2f")) {
                    data.dirty = true;
                }
            } else {
                ImGui::TextDisabled("--");
            }

            ImGui::TableSetColumnIndex(6);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.15f, 0.15f, 1.0f));
            if (ImGui::SmallButton("X")) {
                toDelete = s;
            }
            ImGui::PopStyleColor();

            ImGui::PopID();
        }

        ImGui::EndTable();

        if (toDelete >= 0) {
            wave.spawns.erase(wave.spawns.begin() + toDelete);
            if (data.selectedSpawnIndex == toDelete) {
                data.selectedSpawnIndex = -1;
            } else if (data.selectedSpawnIndex > toDelete) {
                data.selectedSpawnIndex--;
            }
            data.dirty = true;
        }
    }
}

}
