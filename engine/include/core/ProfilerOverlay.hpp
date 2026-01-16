/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** In-game profiler overlay for real-time performance visualization
*/

#ifndef RTYPE_CORE_PROFILER_OVERLAY_HPP
#define RTYPE_CORE_PROFILER_OVERLAY_HPP

#include "core/Profiler.hpp"
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <memory>

namespace rtype {
namespace core {

/**
 * @brief Display mode for the profiler overlay
 */
enum class OverlayMode {
    HIDDEN,     // No overlay shown
    MINIMAL,    // Just FPS counter
    COMPACT,    // FPS + frame time + entity count
    FULL,       // All stats including graphs
    DETAILED    // Full + section breakdowns
};

/**
 * @brief In-game profiler overlay using SFML
 * 
 * Features:
 * - FPS counter with color coding (green/yellow/red)
 * - Frame time display
 * - Entity count
 * - Memory usage
 * - Network latency (if in network mode)
 * - Frame time graph
 * - Section timing breakdown
 * 
 * Toggle with F3, cycle modes with F4
 */
class ProfilerOverlay {
public:
    ProfilerOverlay();
    ~ProfilerOverlay() = default;
    
    /**
     * @brief Initialize the overlay with a font
     * @param fontPath Path to a .ttf font file
     * @return true if initialization succeeded
     */
    bool init(const std::string& fontPath = "");
    
    /**
     * @brief Initialize with an already-loaded font
     * @param font SFML font to use
     */
    void init(const sf::Font& font);
    
    /**
     * @brief Update the overlay (call once per frame)
     */
    void update();
    
    /**
     * @brief Render the overlay to the window
     * @param window SFML RenderWindow to draw to
     */
    void render(sf::RenderWindow& window);
    
    /**
     * @brief Handle input for overlay controls
     * @param event SFML event to process
     * @return true if the event was consumed by the overlay
     */
    bool handleEvent(const sf::Event& event);
    
    // ========================================
    // Mode control
    // ========================================
    void setMode(OverlayMode mode);
    OverlayMode getMode() const;
    void cycleMode();
    void toggle();
    
    // ========================================
    // Appearance
    // ========================================
    void setPosition(float x, float y);
    void setScale(float scale);
    void setOpacity(float opacity); // 0.0 to 1.0
    void setGraphSize(float width, float height);
    
    // ========================================
    // Configuration
    // ========================================
    void setNetworkMode(bool enabled);
    void setShowGraph(bool show);
    void setFPSWarningThreshold(float fps); // Yellow below this
    void setFPSCriticalThreshold(float fps); // Red below this
    
private:
    void updateText();
    void renderMinimal(sf::RenderWindow& window);
    void renderCompact(sf::RenderWindow& window);
    void renderFull(sf::RenderWindow& window);
    void renderDetailed(sf::RenderWindow& window);
    void renderGraph(sf::RenderWindow& window, float x, float y);
    void renderBackground(sf::RenderWindow& window, float width, float height);
    
    sf::Color getFPSColor(double fps) const;
    std::string formatTime(double ms) const;
    std::string formatMemory(size_t bytes) const;
    
private:
    bool _initialized = false;
    OverlayMode _mode = OverlayMode::COMPACT;
    
    // Font and text
    sf::Font _font;
    bool _fontLoaded = false;
    sf::Text _fpsText;
    sf::Text _statsText;
    sf::Text _sectionsText;
    sf::Text _networkText;
    
    // Appearance
    float _posX = 10.0f;
    float _posY = 10.0f;
    float _scale = 1.0f;
    float _opacity = 0.85f;
    float _graphWidth = 200.0f;
    float _graphHeight = 60.0f;
    
    // Configuration
    bool _networkMode = false;
    bool _showGraph = true;
    float _fpsWarning = 45.0f;
    float _fpsCritical = 30.0f;
    
    // Background
    sf::RectangleShape _background;
    
    // Graph
    std::vector<sf::Vertex> _graphVertices;
    sf::RectangleShape _graphBackground;
    sf::RectangleShape _graphLine;
    
    // Colors
    static constexpr uint8_t ALPHA = 220;
    sf::Color _bgColor{20, 20, 20, 200};
    sf::Color _textColor{255, 255, 255, 255};
    sf::Color _fpsGood{100, 255, 100, 255};
    sf::Color _fpsWarning_{255, 200, 50, 255};
    sf::Color _fpsBad{255, 80, 80, 255};
    sf::Color _graphColor{100, 200, 255, 200};
    sf::Color _graphBg{40, 40, 40, 180};
};

} // namespace core
} // namespace rtype

#endif // RTYPE_CORE_PROFILER_OVERLAY_HPP
