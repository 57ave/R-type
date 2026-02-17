#pragma once

#include "StageData.hpp"

namespace Editor {

class WavePanel {
public:
    void Render(EditorData& data);

private:
    bool showDeleteConfirm_ = false;
    int deleteWaveIndex_ = -1;
};

}
