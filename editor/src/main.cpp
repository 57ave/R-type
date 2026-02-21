#include "Editor.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

#include <filesystem>
#include <iostream>
#include <string>

static std::string ResolveScriptsPath(int argc, char* argv[]) {
    if (argc > 1) {
        return argv[1];
    }
    std::vector<std::string> candidates = {
        "../assets/scripts",
        "../../assets/scripts",
        "../../../assets/scripts",
        "assets/scripts",
        "game/assets/scripts",
        "../game/assets/scripts",
        "../../game/assets/scripts",
    };
    for (const auto& p : candidates) {
        if (std::filesystem::exists(p + "/levels/level_1.lua")) {
            return p;
        }
    }
    return "../assets/scripts";
}

int main(int argc, char* argv[]) {
    std::string scriptsPath = ResolveScriptsPath(argc, argv);
    std::string levelsDir = scriptsPath + "/levels";
    std::string enemiesPath = scriptsPath + "/config/enemies_simple.lua";

    std::cout << "[Editor] Scripts path: " << scriptsPath << std::endl;
    std::cout << "[Editor] Levels dir: " << levelsDir << std::endl;
    std::cout << "[Editor] Enemies config: " << enemiesPath << std::endl;

    sf::RenderWindow window(sf::VideoMode(1600, 900), "R-Type Level Editor");
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window)) {
        std::cerr << "[Editor] Failed to initialize ImGui-SFML" << std::endl;
        return 1;
    }

    Editor::EditorApp editor;
    editor.Init(levelsDir, enemiesPath);

    sf::Clock deltaClock;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed) {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        editor.Update(window);

        if (editor.WantsToQuit()) {
            window.close();
        }

        window.clear(sf::Color(40, 40, 40));
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
