#include "Editor.hpp"

#include "LuaParser.hpp"
#include "Serializer.hpp"

#include <imgui.h>

#include <filesystem>
#include <fstream>
#include <iostream>

namespace Editor {

void EditorApp::Init(const std::string& stagesPath, const std::string& enemiesPath) {
    data_.configFilePath = stagesPath;
    data_.enemiesConfigFilePath = enemiesPath;

    namespace fs = std::filesystem;
    fs::path scriptsDir = fs::path(stagesPath).parent_path();
    fs::path projectRoot = scriptsDir.parent_path().parent_path();

    std::vector<fs::path> candidates = {
        projectRoot / "game" / "assets",
        projectRoot / "client" / "assets",
        scriptsDir.parent_path(),
    };
    for (const auto& candidate : candidates) {
        if (fs::exists(candidate / "enemies")) {
            data_.assetsBasePath = candidate.string();
            std::cout << "[Editor] Assets base path: " << data_.assetsBasePath << std::endl;
            break;
        }
    }

    canvas_.Init();

    if (!LuaParser::LoadEnemies(enemiesPath, data_.enemyTypes)) {
        errorMessage_ = "Failed to load enemies: " + LuaParser::GetLastError();
        showError_ = true;
        std::cerr << "[Editor] " << errorMessage_ << std::endl;
    }

    if (!LuaParser::LoadStages(stagesPath, data_.stages, helperFunctionsBlock_)) {
        errorMessage_ = "Failed to load stages: " + LuaParser::GetLastError();
        showError_ = true;
        std::cerr << "[Editor] " << errorMessage_ << std::endl;
    }

    AssignSpawnIds();
    data_.dirty = false;

    std::cout << "[Editor] Initialized with " << data_.stages.size() << " stages, "
              << data_.enemyTypes.size() << " enemy types" << std::endl;
}

void EditorApp::AssignSpawnIds() {
    data_.nextSpawnId = 1;
    for (auto& stage : data_.stages) {
        for (auto& wave : stage.waves) {
            for (auto& spawn : wave.spawns) {
                spawn.editorId = data_.nextSpawnId++;
            }
        }
    }
}

void EditorApp::Update(sf::RenderWindow& window) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float statusHeight = ImGui::GetFrameHeight();
    float totalW = viewport->WorkSize.x;
    float totalH = viewport->WorkSize.y;

    if (ImGui::BeginMainMenuBar()) {
        RenderMenuBar();
        ImGui::EndMainMenuBar();
    }

    float workY = viewport->WorkPos.y;
    float workH = totalH - statusHeight;

    float leftW = 350.0f;
    float rightW = totalW - leftW;
    float canvasH = workH * 0.55f;
    float spawnsH = workH - canvasH;
    float stageH = workH * 0.45f;
    float wavesH = workH - stageH;

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, workY));
    ImGui::SetNextWindowSize(ImVec2(leftW, stageH));
    ImGui::Begin("Stage", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    RenderStageSelector();
    RenderStageProperties();
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, workY + stageH));
    ImGui::SetNextWindowSize(ImVec2(leftW, wavesH));
    ImGui::Begin("Waves", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    wavePanel_.Render(data_);
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + leftW, workY));
    ImGui::SetNextWindowSize(ImVec2(rightW, canvasH));
    ImGui::Begin("Canvas", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    canvas_.Render(data_, window);
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + leftW, workY + canvasH));
    ImGui::SetNextWindowSize(ImVec2(rightW, spawnsH));
    ImGui::Begin("Spawns", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    spawnTable_.Render(data_);
    ImGui::End();

    RenderStatusBar();

    RenderErrorPopup();

    auto& io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        Save();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_Delete) && data_.selectedSpawnIndex >= 0) {
        if (!data_.stages.empty()) {
            auto& stage = data_.stages[data_.selectedStageIndex];
            if (data_.selectedWaveIndex >= 0 &&
                data_.selectedWaveIndex < static_cast<int>(stage.waves.size())) {
                auto& wave = stage.waves[data_.selectedWaveIndex];
                if (data_.selectedSpawnIndex < static_cast<int>(wave.spawns.size())) {
                    wave.spawns.erase(wave.spawns.begin() + data_.selectedSpawnIndex);
                    data_.selectedSpawnIndex = -1;
                    data_.dirty = true;
                }
            }
        }
    }
}

void EditorApp::RenderMenuBar() {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Save", "Ctrl+S")) {
            Save();
        }
        if (ImGui::MenuItem("Reload")) {
            Reload();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Quit")) {
            wantsToQuit_ = true;
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Edit")) {
        if (ImGui::MenuItem("Add New Stage")) {
            StageData newStage;
            newStage.key = "stage" + std::to_string(static_cast<int>(data_.stages.size()) + 1);
            newStage.name = "New Stage";
            newStage.stageNumber = static_cast<int>(data_.stages.size()) + 1;
            data_.stages.push_back(newStage);
            data_.selectedStageIndex = static_cast<int>(data_.stages.size()) - 1;
            data_.dirty = true;
        }
        ImGui::EndMenu();
    }

    if (data_.dirty) {
        ImGui::SameLine(ImGui::GetWindowWidth() - 150);
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[Unsaved Changes]");
    }
}

void EditorApp::RenderStageSelector() {
    if (data_.stages.empty()) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "No stages loaded.");
        return;
    }

    if (ImGui::BeginTabBar("StageTabs")) {
        for (int i = 0; i < static_cast<int>(data_.stages.size()); i++) {
            std::string label =
                "Stage " + std::to_string(data_.stages[i].stageNumber) + ": " + data_.stages[i].name;
            if (ImGui::BeginTabItem(label.c_str())) {
                if (data_.selectedStageIndex != i) {
                    data_.selectedStageIndex = i;
                    data_.selectedWaveIndex = 0;
                    data_.selectedSpawnIndex = -1;
                }
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

void EditorApp::RenderStageProperties() {
    if (data_.stages.empty()) return;
    auto& stage = data_.stages[data_.selectedStageIndex];

    ImGui::Separator();
    ImGui::Text("Stage Properties");
    ImGui::Separator();

    char nameBuf[128];
    std::snprintf(nameBuf, sizeof(nameBuf), "%s", stage.name.c_str());
    if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
        stage.name = nameBuf;
        data_.dirty = true;
    }

    char descBuf[256];
    std::snprintf(descBuf, sizeof(descBuf), "%s", stage.description.c_str());
    if (ImGui::InputText("Description", descBuf, sizeof(descBuf))) {
        stage.description = descBuf;
        data_.dirty = true;
    }

    if (ImGui::DragFloat("Duration (s)", &stage.duration, 1.0f, 10.0f, 600.0f, "%.1f")) {
        data_.dirty = true;
    }

    ImGui::Separator();
    ImGui::Text("Background");

    char bgBuf[128];
    std::snprintf(bgBuf, sizeof(bgBuf), "%s", stage.background.texture.c_str());
    if (ImGui::InputText("Texture", bgBuf, sizeof(bgBuf))) {
        stage.background.texture = bgBuf;
        data_.dirty = true;
    }
    if (ImGui::DragFloat("Scroll Speed", &stage.background.scrollSpeed, 1.0f, 0.0f, 1000.0f,
                         "%.0f")) {
        data_.dirty = true;
    }

    ImGui::Separator();
    ImGui::Text("Bonuses");

    if (ImGui::InputInt("Completion Bonus", &stage.completionBonus, 500)) {
        data_.dirty = true;
    }
    if (ImGui::InputInt("Perfect Bonus", &stage.perfectBonus, 1000)) {
        data_.dirty = true;
    }
    if (ImGui::DragFloat("Speed Bonus Time", &stage.speedBonusTime, 1.0f, 0.0f, 600.0f,
                         "%.1f")) {
        data_.dirty = true;
    }
    if (ImGui::InputInt("Speed Bonus", &stage.speedBonus, 500)) {
        data_.dirty = true;
    }
}

void EditorApp::RenderStatusBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float statusHeight = ImGui::GetFrameHeight();
    ImGui::SetNextWindowPos(
        ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + viewport->WorkSize.y - statusHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, statusHeight));

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse |
                             ImGuiWindowFlags_NoBringToFrontOnFocus;

    ImGui::Begin("##StatusBar", nullptr, flags);

    ImGui::Text("Stages: %d", static_cast<int>(data_.stages.size()));
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    if (!data_.stages.empty()) {
        auto& stage = data_.stages[data_.selectedStageIndex];
        ImGui::Text("Waves: %d", static_cast<int>(stage.waves.size()));
        ImGui::SameLine();
        ImGui::Text(" | ");
        ImGui::SameLine();

        if (data_.selectedWaveIndex >= 0 &&
            data_.selectedWaveIndex < static_cast<int>(stage.waves.size())) {
            auto& wave = stage.waves[data_.selectedWaveIndex];
            ImGui::Text("Spawns: %d", static_cast<int>(wave.spawns.size()));
        }
    }

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();
    ImGui::Text("%s", data_.configFilePath.c_str());

    ImGui::End();
}

void EditorApp::RenderErrorPopup() {
    if (showError_) {
        ImGui::OpenPopup("Error");
        showError_ = false;
    }

    if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped("%s", errorMessage_.c_str());
        ImGui::Separator();
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void EditorApp::Save() {
    std::string content = Serializer::SerializeStages(data_.stages, helperFunctionsBlock_);

    std::ofstream file(data_.configFilePath);
    if (!file.is_open()) {
        errorMessage_ = "Cannot write to: " + data_.configFilePath;
        showError_ = true;
        return;
    }

    file << content;
    file.close();

    data_.dirty = false;
    std::cout << "[Editor] Saved to " << data_.configFilePath << std::endl;
}

void EditorApp::SaveAs(const std::string& path) {
    std::string oldPath = data_.configFilePath;
    data_.configFilePath = path;
    Save();
    if (data_.dirty) {
        data_.configFilePath = oldPath;
    }
}

void EditorApp::Reload() {
    data_.stages.clear();
    if (!LuaParser::LoadStages(data_.configFilePath, data_.stages, helperFunctionsBlock_)) {
        errorMessage_ = "Failed to reload: " + LuaParser::GetLastError();
        showError_ = true;
    }
    data_.selectedStageIndex = 0;
    data_.selectedWaveIndex = 0;
    data_.selectedSpawnIndex = -1;
    AssignSpawnIds();
    data_.dirty = false;
}

}
