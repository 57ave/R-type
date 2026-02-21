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
    void Init(const std::string& stagesPath, const std::string& enemiesPath);
    void Update(sf::RenderWindow& window);

    bool WantsToQuit() const { return wantsToQuit_; }
    const EditorData& GetData() const { return data_; }

private:
    EditorData data_;
    WavePanel wavePanel_;
    SpawnTable spawnTable_;
    Canvas canvas_;

    std::string helperFunctionsBlock_;
    bool wantsToQuit_ = false;
    bool showSaveConfirm_ = false;
    std::string errorMessage_;
    bool showError_ = false;

    void RenderMenuBar();
    void RenderStageSelector();
    void RenderStageProperties();
    void RenderStatusBar();
    void RenderErrorPopup();

    void Save();
    void SaveAs(const std::string& path);
    void Reload();

    void AssignSpawnIds();
};

}
