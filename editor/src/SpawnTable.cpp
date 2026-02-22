#include "SpawnTable.hpp"

#include <imgui.h>

#include <cstdio>
#include <string>

namespace Editor {

void SpawnTable::Render(EditorData& data) {
    if (data.levels.empty()) return;

    auto& level = data.levels[data.selectedLevelIndex];

    ImGui::Text("Wave Overview - Level %d: %s", level.id, level.name.c_str());
    ImGui::Separator();

    if (level.waves.empty()) {
        ImGui::TextColored(ImVec4(1, 0.8f, 0.4f, 1), "No waves defined. Add waves in the Waves panel.");
        return;
    }

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg |
                                 ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY |
                                 ImGuiTableFlags_SizingStretchProp;

    if (ImGui::BeginTable("WaveOverview", 5, tableFlags,
                          ImVec2(0.0f, ImGui::GetContentRegionAvail().y))) {
        ImGui::TableSetupColumn("Wave", ImGuiTableColumnFlags_WidthFixed, 60.0f);
        ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Enemies", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Total", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Duration", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();

        for (int w = 0; w < static_cast<int>(level.waves.size()); w++) {
            auto& wave = level.waves[w];
            ImGui::PushID(w);
            ImGui::TableNextRow();

            bool isSelected = (data.selectedWaveIndex == w);
            if (isSelected) {
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg1,
                                       ImGui::GetColorU32(ImVec4(0.2f, 0.3f, 0.6f, 0.4f)));
            }

            // Wave number
            ImGui::TableSetColumnIndex(0);
            char label[32];
            std::snprintf(label, sizeof(label), "W%d", w + 1);
            if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_SpanAllColumns)) {
                data.selectedWaveIndex = w;
            }

            // Time
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.1fs", wave.time);

            // Enemies breakdown
            ImGui::TableSetColumnIndex(2);
            std::string enemyStr;
            int totalCount = 0;
            float maxDuration = 0.0f;
            for (const auto& e : wave.enemies) {
                if (!enemyStr.empty()) enemyStr += ", ";
                std::string typeName = (e.type >= 0 && e.type < EnemyTypeNameCount)
                                           ? EnemyTypeNames[e.type]
                                           : "type" + std::to_string(e.type);
                char buf[64];
                std::snprintf(buf, sizeof(buf), "%dx %s", e.count, typeName.c_str());
                enemyStr += buf;
                totalCount += e.count;
                float dur = e.count * e.interval;
                if (dur > maxDuration) maxDuration = dur;
            }
            ImGui::TextUnformatted(enemyStr.c_str());

            // Total enemies
            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%d", totalCount);

            // Estimated duration
            ImGui::TableSetColumnIndex(4);
            ImGui::Text("~%.1fs", maxDuration);

            ImGui::PopID();
        }

        ImGui::EndTable();
    }

    // Boss info at the bottom
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
                       "Boss: %s (type %d) spawns at %.1fs",
                       level.boss.name.c_str(), level.boss.type, level.boss.spawnTime);
}

}
