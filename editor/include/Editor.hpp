#pragma once

#include "Canvas.hpp"
#include "SpawnTable.hpp"
#include "StageData.hpp"
#include "WavePanel.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <string>

namespace Editor {

class EditorApp {
public:
    void Init(const std::string& levelsDir, const std::string& enemiesPath);
    void Update(sf::RenderWindow& window);

    bool WantsToQuit() const { return wantsToQuit_; }
    const EditorData& GetData() const { return data_; }

private:
    EditorData data_;
    WavePanel wavePanel_;
    SpawnTable spawnTable_;
    Canvas canvas_;

    bool wantsToQuit_ = false;
    std::string errorMessage_;
    bool showError_ = false;

    void RenderMenuBar();
    void RenderLevelSelector();
    void RenderLevelProperties();
    void RenderBossProperties();
    void RenderStatusBar();
    void RenderErrorPopup();

    void Save();
    void Reload();
    void CopyLevelsToBuildDirs();
};

}
