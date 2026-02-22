#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace Editor {

class FolderBrowser {
public:
    void Open(const std::string& startPath);
    void Render();

    bool HasResult() const { return hasResult_; }
    std::string ConsumeResult();

private:
    bool showPopup_ = false;
    bool hasResult_ = false;
    std::string currentPath_;
    std::string result_;
};

}
