/*
** EPITECH PROJECT, 2025
** Config
** File description:
** engine
*/

#include "core/Config.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace eng {
namespace core {

Config::Config()
{
}

Config::~Config()
{
    _data.clear();
}

// Extrait le nom du fichier sans extension
std::string Config::getFilenameWithoutExtension(const std::string& filepath) const
{
    std::filesystem::path p(filepath);
    return p.stem().string();
}

// Scanne un dossier et ajoute tous les .gif dans _data
void Config::scanDirectory(const std::string& path, const std::string& category)
{
    try {
        if (!std::filesystem::exists(path)) {
            std::cerr << "Directory does not exist: " << path << std::endl;
            return;
        }

        std::cout << "Scanning " << category << ": " << path << std::endl;

        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();

                if (ext == ".gif") {
                    std::string filename = getFilenameWithoutExtension(entry.path().string());
                    std::string key = category + "." + filename;
                    std::string value = entry.path().string();
                    _data[key] = value;
                    std::cout << "  + " << key << " -> " << value << std::endl;
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error scanning directory: " << e.what() << std::endl;
    }
}

// Charge la configuration depuis un dossier d'assets
void Config::load(const std::string& filepath)
{
    std::cout << "========================================" << std::endl;
    std::cout << "Loading assets from: " << filepath << std::endl;
    std::cout << "========================================" << std::endl;

    // Scanne le dossier players
    std::string playersPath = filepath + "/players";
    scanDirectory(playersPath, "players");

    // Scanne le dossier enemies
    std::string enemiesPath = filepath + "/enemies";
    scanDirectory(enemiesPath, "enemies");

    std::cout << "\n[Success] Total assets loaded: " << _data.size() << std::endl;
    std::cout << "========================================\n" << std::endl;
}

// Sauvegarde la configuration dans un fichier
void Config::save(const std::string& filepath) const
{
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to save config to: " << filepath << std::endl;
        return;
    }
    
    file << "# Game Asset Configuration" << std::endl;
    file << "# Auto-generated file" << std::endl;
    file << std::endl;
    
    for (const auto& [key, value] : _data) {
        file << key << " = " << value << std::endl;
    }
    
    file.close();
    std::cout << "Config saved to: " << filepath << std::endl;
}

} // namespace core
} // namespace eng