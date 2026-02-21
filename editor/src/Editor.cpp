#include "Editor.hpp"

#include "LuaParser.hpp"
#include "Serializer.hpp"

#include <imgui.h>

#include <algorithm>
#include <filesystem>
#include <iostream>

namespace Editor {

void EditorApp::Init(const std::string& levelsDir, const std::string& enemiesPath) {
    data_.levelsDir = levelsDir;
    data_.enemiesConfigPath = enemiesPath;

    namespace fs = std::filesystem;
    fs::path levelsPath(levelsDir);
    fs::path projectRoot = levelsPath.parent_path().parent_path().parent_path();

    std::vector<fs::path> candidates = {
        projectRoot / "game" / "assets",
        levelsPath.parent_path().parent_path(),
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

    if (!LuaParser::LoadLevels(levelsDir, data_.levels)) {
        errorMessage_ = "Failed to load levels: " + LuaParser::GetLastError();
        showError_ = true;
        std::cerr << "[Editor] " << errorMessage_ << std::endl;
    }

    data_.dirty = false;

    std::cout << "[Editor] Initialized with " << data_.levels.size() << " levels, "
              << data_.enemyTypes.size() << " enemy types" << std::endl;
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

    float leftW = 380.0f;
    float rightW = totalW - leftW;
    float topH = workH * 0.50f;
    float bottomH = workH - topH;

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, workY));
    ImGui::SetNextWindowSize(ImVec2(leftW, topH));
    ImGui::Begin("Level", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    RenderLevelSelector();
    RenderLevelProperties();
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, workY + topH));
    ImGui::SetNextWindowSize(ImVec2(leftW, bottomH));
    ImGui::Begin("Boss", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    RenderBossProperties();
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + leftW, workY));
    ImGui::SetNextWindowSize(ImVec2(rightW, topH));
    ImGui::Begin("Canvas", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    canvas_.Render(data_, window);
    ImGui::End();

    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + leftW, workY + topH));
    ImGui::SetNextWindowSize(ImVec2(rightW, bottomH));
    ImGui::Begin("Waves", nullptr,
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    wavePanel_.Render(data_);
    ImGui::End();

    RenderStatusBar();
    RenderErrorPopup();

    auto& io = ImGui::GetIO();
    if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        Save();
    }
}

void EditorApp::RenderMenuBar() {
    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Save All", "Ctrl+S")) {
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
        if (ImGui::MenuItem("Add New Level")) {
            LevelData newLevel;
            newLevel.id = static_cast<int>(data_.levels.size()) + 1;
            newLevel.name = "New Level";
            newLevel.enemyTypes = {0};
            newLevel.moduleTypes = {3, 4};
            newLevel.filePath = data_.levelsDir + "/level_" +
                                std::to_string(newLevel.id) + ".lua";
            data_.levels.push_back(newLevel);
            data_.selectedLevelIndex = static_cast<int>(data_.levels.size()) - 1;
            data_.dirty = true;
        }
        ImGui::EndMenu();
    }

    if (data_.dirty) {
        ImGui::SameLine(ImGui::GetWindowWidth() - 150);
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "[Unsaved Changes]");
    }
}

void EditorApp::RenderLevelSelector() {
    if (data_.levels.empty()) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "No levels loaded.");
        return;
    }

    if (ImGui::BeginTabBar("LevelTabs")) {
        for (int i = 0; i < static_cast<int>(data_.levels.size()); i++) {
            std::string label = "Level " + std::to_string(data_.levels[i].id) +
                                ": " + data_.levels[i].name;
            if (ImGui::BeginTabItem(label.c_str())) {
                if (data_.selectedLevelIndex != i) {
                    data_.selectedLevelIndex = i;
                    data_.selectedWaveIndex = 0;
                }
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
}

void EditorApp::RenderLevelProperties() {
    if (data_.levels.empty()) return;
    auto& level = data_.levels[data_.selectedLevelIndex];

    ImGui::Separator();
    ImGui::Text("Level Properties");
    ImGui::Separator();

    char nameBuf[128];
    std::snprintf(nameBuf, sizeof(nameBuf), "%s", level.name.c_str());
    if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
        level.name = nameBuf;
        data_.dirty = true;
    }

    if (ImGui::InputInt("Level ID", &level.id)) {
        data_.dirty = true;
    }

    ImGui::Separator();
    ImGui::Text("Enemy Types");
    for (int et = 0; et < EnemyTypeNameCount; et++) {
        bool has = false;
        for (int v : level.enemyTypes) {
            if (v == et) { has = true; break; }
        }
        if (ImGui::Checkbox(EnemyTypeNames[et], &has)) {
            if (has) {
                level.enemyTypes.push_back(et);
            } else {
                level.enemyTypes.erase(
                    std::remove(level.enemyTypes.begin(), level.enemyTypes.end(), et),
                    level.enemyTypes.end());
            }
            data_.dirty = true;
        }
        if (et < EnemyTypeNameCount - 1) ImGui::SameLine();
    }

    ImGui::Separator();
    ImGui::Text("Module Types");
    int moduleIds[] = {1, 3, 4};
    const char* moduleNames[] = {"homing", "spread", "wave"};
    for (int m = 0; m < 3; m++) {
        int mid = moduleIds[m];
        bool has = false;
        for (int v : level.moduleTypes) {
            if (v == mid) { has = true; break; }
        }
        if (ImGui::Checkbox(moduleNames[m], &has)) {
            if (has) {
                level.moduleTypes.push_back(mid);
            } else {
                level.moduleTypes.erase(
                    std::remove(level.moduleTypes.begin(), level.moduleTypes.end(), mid),
                    level.moduleTypes.end());
            }
            data_.dirty = true;
        }
        if (m < 2) ImGui::SameLine();
    }

    ImGui::Separator();
    ImGui::Text("Spawn Configuration");
    if (ImGui::DragFloat("Enemy Interval", &level.spawn.enemyInterval, 0.1f, 0.1f, 30.0f, "%.1f")) {
        data_.dirty = true;
    }
    if (ImGui::DragFloat("Powerup Interval", &level.spawn.powerupInterval, 0.5f, 1.0f, 60.0f, "%.1f")) {
        data_.dirty = true;
    }
    if (ImGui::DragFloat("Module Interval", &level.spawn.moduleInterval, 0.5f, 1.0f, 60.0f, "%.1f")) {
        data_.dirty = true;
    }
    if (ImGui::InputInt("Max Enemies", &level.spawn.maxEnemies)) {
        if (level.spawn.maxEnemies < 1) level.spawn.maxEnemies = 1;
        data_.dirty = true;
    }

    if (ImGui::Checkbox("Stop Spawning At Boss", &level.stopSpawningAtBoss)) {
        data_.dirty = true;
    }
}

void EditorApp::RenderBossProperties() {
    if (data_.levels.empty()) return;
    auto& boss = data_.levels[data_.selectedLevelIndex].boss;

    ImGui::Text("Boss Configuration");
    ImGui::Separator();

    char nameBuf[128];
    std::snprintf(nameBuf, sizeof(nameBuf), "%s", boss.name.c_str());
    if (ImGui::InputText("Boss Name", nameBuf, sizeof(nameBuf))) {
        boss.name = nameBuf;
        data_.dirty = true;
    }

    if (ImGui::InputInt("Boss Type ID", &boss.type)) {
        data_.dirty = true;
    }
    if (ImGui::InputInt("Health", &boss.health)) {
        data_.dirty = true;
    }
    if (ImGui::DragFloat("Speed", &boss.speed, 1.0f, 0.0f, 500.0f, "%.1f")) {
        data_.dirty = true;
    }
    if (ImGui::DragFloat("Spawn Time", &boss.spawnTime, 1.0f, 0.0f, 600.0f, "%.1f")) {
        data_.dirty = true;
    }
    if (ImGui::DragFloat("Fire Rate", &boss.fireRate, 0.1f, 0.1f, 10.0f, "%.1f")) {
        data_.dirty = true;
    }

    int patternIdx = 0;
    for (int i = 0; i < FirePatternCount; i++) {
        if (FirePatternValues[i] == boss.firePattern) {
            patternIdx = i;
            break;
        }
    }
    if (ImGui::Combo("Fire Pattern", &patternIdx, FirePatternNames, FirePatternCount)) {
        boss.firePattern = FirePatternValues[patternIdx];
        data_.dirty = true;
    }

    ImGui::Separator();
    ImGui::Text("Boss Sprite");

    char spriteBuf[256];
    std::snprintf(spriteBuf, sizeof(spriteBuf), "%s", boss.sprite.path.c_str());
    if (ImGui::InputText("Sprite Path", spriteBuf, sizeof(spriteBuf))) {
        boss.sprite.path = spriteBuf;
        data_.dirty = true;
    }
    if (ImGui::InputInt("Frame Width", &boss.sprite.frameWidth)) {
        data_.dirty = true;
    }
    if (ImGui::InputInt("Frame Height", &boss.sprite.frameHeight)) {
        data_.dirty = true;
    }
    if (ImGui::InputInt("Frame Count", &boss.sprite.frameCount)) {
        data_.dirty = true;
    }
    if (ImGui::DragFloat("Frame Time", &boss.sprite.frameTime, 0.01f, 0.01f, 1.0f, "%.2f")) {
        data_.dirty = true;
    }
    if (ImGui::DragFloat("Scale", &boss.sprite.scale, 0.1f, 0.1f, 5.0f, "%.1f")) {
        data_.dirty = true;
    }
    if (ImGui::Checkbox("Vertical Spritesheet", &boss.sprite.vertical)) {
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

    ImGui::Text("Levels: %d", static_cast<int>(data_.levels.size()));
    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();

    if (!data_.levels.empty()) {
        auto& level = data_.levels[data_.selectedLevelIndex];
        ImGui::Text("Waves: %d", static_cast<int>(level.waves.size()));
        ImGui::SameLine();
        ImGui::Text(" | ");
        ImGui::SameLine();
        ImGui::Text("Boss: %s", level.boss.name.c_str());
    }

    ImGui::SameLine();
    ImGui::Text(" | ");
    ImGui::SameLine();
    ImGui::Text("%s", data_.levelsDir.c_str());

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

void EditorApp::CopyLevelsToBuildDirs() {
    namespace fs = std::filesystem;

    fs::path levelsPath = fs::canonical(fs::path(data_.levelsDir));
    // Walk up to find the project root (contains build/)
    fs::path projectRoot;
    for (fs::path p = levelsPath; p.has_parent_path() && p != p.parent_path(); p = p.parent_path()) {
        if (fs::exists(p / "build") && fs::exists(p / "game")) {
            projectRoot = p;
            break;
        }
    }
    if (projectRoot.empty()) return;

    // Find all build dirs that have a game/assets/scripts/levels/ path
    std::vector<fs::path> buildLevelDirs;
    fs::path buildDir = projectRoot / "build";
    if (!fs::exists(buildDir)) return;

    for (const auto& entry : fs::recursive_directory_iterator(buildDir)) {
        if (entry.is_directory() && entry.path().filename() == "levels") {
            fs::path parent = entry.path().parent_path();
            if (parent.filename() == "scripts") {
                buildLevelDirs.push_back(entry.path());
            }
        }
    }

    // Copy each level file to each build levels dir
    for (const auto& level : data_.levels) {
        if (level.filePath.empty()) continue;
        fs::path srcFile(level.filePath);
        if (!fs::exists(srcFile)) continue;

        for (const auto& destDir : buildLevelDirs) {
            fs::path destFile = destDir / srcFile.filename();
            try {
                fs::copy_file(srcFile, destFile, fs::copy_options::overwrite_existing);
                std::cout << "[Editor] Copied " << srcFile.filename() << " -> " << destDir << std::endl;
            } catch (const fs::filesystem_error& e) {
                std::cerr << "[Editor] Failed to copy to build: " << e.what() << std::endl;
            }
        }
    }
}

void EditorApp::Save() {
    for (auto& level : data_.levels) {
        std::string path = level.filePath;
        if (path.empty()) {
            path = data_.levelsDir + "/level_" + std::to_string(level.id) + ".lua";
            level.filePath = path;
        }
        if (!Serializer::SaveLevel(level, path)) {
            errorMessage_ = "Failed to save level " + std::to_string(level.id);
            showError_ = true;
            return;
        }
    }

    CopyLevelsToBuildDirs();

    data_.dirty = false;
    std::cout << "[Editor] All levels saved" << std::endl;
}

void EditorApp::Reload() {
    data_.levels.clear();
    if (!LuaParser::LoadLevels(data_.levelsDir, data_.levels)) {
        errorMessage_ = "Failed to reload: " + LuaParser::GetLastError();
        showError_ = true;
    }
    data_.selectedLevelIndex = 0;
    data_.selectedWaveIndex = 0;
    data_.dirty = false;
}

}
