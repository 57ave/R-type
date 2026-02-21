#include "Editor.hpp"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <imgui-SFML.h>
#include <imgui.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

static std::string ResolveAssetsPath(int argc, char* argv[]) {
    if (argc > 1) {
        return argv[1];
    }
    std::vector<std::string> candidates = {
        "../assets/scripts",
        "../../assets/scripts",
        "../../../assets/scripts",
        "assets/scripts",
    };
    for (const auto& p : candidates) {
        if (std::filesystem::exists(p + "/stages_config.lua")) {
            return p;
        }
    }
    return "../assets/scripts";
}

int main(int argc, char* argv[]) {
    std::string assetsPath = ResolveAssetsPath(argc, argv);
    std::cout << "[Editor] Assets path: " << assetsPath << std::endl;

    sf::RenderWindow window(sf::VideoMode(1600, 900), "R-Type Level Editor");
    window.setFramerateLimit(60);

    if (!ImGui::SFML::Init(window)) {
        std::cerr << "[Editor] Failed to initialize ImGui-SFML" << std::endl;
        return 1;
    }

    Editor::EditorApp editor;
    editor.Init(assetsPath + "/stages_config.lua", assetsPath + "/enemies_config.lua");

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
