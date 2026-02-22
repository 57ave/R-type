#include "FolderBrowser.hpp"

#include <imgui.h>

#include <algorithm>
#include <iostream>

namespace Editor {

void FolderBrowser::Open(const std::string& startPath) {
    namespace fs = std::filesystem;
    showPopup_ = true;
    hasResult_ = false;
    try {
        fs::path p = startPath.empty() ? fs::current_path() : fs::path(startPath);
        currentPath_ = fs::canonical(p).string();
    } catch (const fs::filesystem_error&) {
        currentPath_ = fs::current_path().string();
    }
}

std::string FolderBrowser::ConsumeResult() {
    hasResult_ = false;
    return std::move(result_);
}

void FolderBrowser::Render() {
    namespace fs = std::filesystem;

    if (showPopup_) {
        ImGui::OpenPopup("Select Assets Folder");
        showPopup_ = false;
    }

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    if (!ImGui::BeginPopupModal("Select Assets Folder", nullptr, ImGuiWindowFlags_NoScrollbar))
        return;

    fs::path current(currentPath_);
    bool atRoot = current == current.root_path();
    if (atRoot) ImGui::BeginDisabled();
    if (ImGui::Button("..")) {
        auto parent = current.parent_path();
        if (!parent.empty() && fs::exists(parent)) {
            currentPath_ = fs::canonical(parent).string();
        }
    }
    if (atRoot) ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Text("%s", currentPath_.c_str());
    ImGui::Separator();

    ImGui::BeginChild("FolderList", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 2), true);
    try {
        std::vector<fs::path> dirs;
        for (const auto& entry : fs::directory_iterator(currentPath_)) {
            if (entry.is_directory() && entry.path().filename().string()[0] != '.') {
                dirs.push_back(entry.path());
            }
        }
        std::sort(dirs.begin(), dirs.end());
        for (const auto& dir : dirs) {
            std::string label = dir.filename().string() + "/";
            if (ImGui::Selectable(label.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                if (ImGui::IsMouseDoubleClicked(0)) {
                    currentPath_ = dir.string();
                }
            }
        }
    } catch (const fs::filesystem_error& e) {
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "Cannot read directory");
        ImGui::TextColored(ImVec4(1, 0.4f, 0.4f, 1), "%s", e.what());
    }
    ImGui::EndChild();

    ImGui::Separator();
    if (ImGui::Button("Select This Folder", ImVec2(150, 0))) {
        result_ = currentPath_;
        hasResult_ = true;
        ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(100, 0))) {
        ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
}

}
