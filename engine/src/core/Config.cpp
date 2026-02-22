/*
** EPITECH PROJECT, 2025
** Config
** File description:
** engine
*/

#include "core/Config.hpp"
#include <filesystem>
#include <fstream>
#include "core/Logger.hpp"

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
            LOG_ERROR("CONFIG", "Directory does not exist: " + path);
            return;
        }

        LOG_INFO("CONFIG", "Scanning " + std::to_string(category) + ": " + path);

        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::string ext = entry.path().extension().string();

                if (ext == ".gif") {
                    std::string filename = getFilenameWithoutExtension(entry.path().string());
                    std::string key = category + "." + filename;
                    std::string value = entry.path().string();
                    _data[key] = value;
                    LOG_INFO("CONFIG", "  + " + key + " -> " + std::to_string(value));
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR("CONFIG", "Error scanning directory: " + std::string(e.what()));
    }
}

// Charge la configuration depuis un dossier d'assets
void Config::load(const std::string& filepath)
{
    LOG_INFO("CONFIG", "========================================");
    LOG_INFO("CONFIG", "Loading assets from: " + filepath);
    LOG_INFO("CONFIG", "========================================");

    // Scanne le dossier players
    std::string playersPath = filepath + "/players";
    scanDirectory(playersPath, "players");

    // Scanne le dossier enemies
    std::string enemiesPath = filepath + "/enemies";
    scanDirectory(enemiesPath, "enemies");

    LOG_INFO("CONFIG", "\n[Success] Total assets loaded: " + std::to_string(_data.size()));
    LOG_INFO("CONFIG", "========================================\n");
}

// Sauvegarde la configuration dans un fichier
void Config::save(const std::string& filepath) const
{
    std::ofstream file(filepath);
    if (!file.is_open()) {
        LOG_ERROR("CONFIG", "Failed to save config to: " + filepath);
        return;
    }
    
    file << "# Game Asset Configuration" << std::endl;
    file << "# Auto-generated file" << std::endl;
    file << std::endl;
    
    for (const auto& [key, value] : _data) {
        file << key << " = " << value << std::endl;
    }
    
    file.close();
    LOG_INFO("CONFIG", "Config saved to: " + filepath);
}

} // namespace core
} // namespace eng