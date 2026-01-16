/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** In-game profiler overlay implementation
*/

#include "core/ProfilerOverlay.hpp"
#include "core/Logger.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace rtype {
namespace core {

ProfilerOverlay::ProfilerOverlay()
{
    _background.setFillColor(_bgColor);
    _graphBackground.setFillColor(_graphBg);
}

bool ProfilerOverlay::init(const std::string& fontPath)
{
    if (fontPath.empty()) {
        // Try common system font locations
        std::vector<std::string> fontPaths = {
            // macOS
            "/System/Library/Fonts/Menlo.ttc",
            "/System/Library/Fonts/Monaco.ttf",
            "/System/Library/Fonts/SFNSMono.ttf",
            "/Library/Fonts/Arial.ttf",
            // Linux
            "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf",
            "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
            // Windows
            "C:/Windows/Fonts/consola.ttf",
            "C:/Windows/Fonts/arial.ttf",
            // Relative paths
            "assets/fonts/arial.ttf",
            "../assets/fonts/arial.ttf",
            "../../assets/fonts/arial.ttf"
        };
        
        for (const auto& path : fontPaths) {
            if (_font.loadFromFile(path)) {
                _fontLoaded = true;
                LOG_DEBUG("PROFILER", "Loaded font from: " + path);
                break;
            }
        }
        
        if (!_fontLoaded) {
            LOG_WARNING("PROFILER", "Could not load any system font - overlay text disabled");
            _initialized = true; // Still initialize, just without text
            return true;
        }
    } else {
        if (!_font.loadFromFile(fontPath)) {
            LOG_ERROR("PROFILER", "Failed to load font: " + fontPath);
            return false;
        }
        _fontLoaded = true;
    }
    
    // Initialize text objects
    if (_fontLoaded) {
        unsigned int fontSize = static_cast<unsigned int>(14 * _scale);
        
        _fpsText.setFont(_font);
        _fpsText.setCharacterSize(fontSize + 4);
        _fpsText.setFillColor(_textColor);
        
        _statsText.setFont(_font);
        _statsText.setCharacterSize(fontSize);
        _statsText.setFillColor(_textColor);
        
        _sectionsText.setFont(_font);
        _sectionsText.setCharacterSize(fontSize - 2);
        _sectionsText.setFillColor(_textColor);
        
        _networkText.setFont(_font);
        _networkText.setCharacterSize(fontSize);
        _networkText.setFillColor(_textColor);
    }
    
    _initialized = true;
    LOG_INFO("PROFILER", "ProfilerOverlay initialized");
    return true;
}

void ProfilerOverlay::init(const sf::Font& font)
{
    _font = font;
    _fontLoaded = true;
    
    unsigned int fontSize = static_cast<unsigned int>(14 * _scale);
    
    _fpsText.setFont(_font);
    _fpsText.setCharacterSize(fontSize + 4);
    _fpsText.setFillColor(_textColor);
    
    _statsText.setFont(_font);
    _statsText.setCharacterSize(fontSize);
    _statsText.setFillColor(_textColor);
    
    _sectionsText.setFont(_font);
    _sectionsText.setCharacterSize(fontSize - 2);
    _sectionsText.setFillColor(_textColor);
    
    _networkText.setFont(_font);
    _networkText.setCharacterSize(fontSize);
    _networkText.setFillColor(_textColor);
    
    _initialized = true;
}

void ProfilerOverlay::update()
{
    if (!_initialized || _mode == OverlayMode::HIDDEN) {
        return;
    }
    
    updateText();
}

void ProfilerOverlay::updateText()
{
    auto& profiler = Profiler::getInstance();
    
    // FPS text with color
    double fps = profiler.getCurrentFPS();
    std::ostringstream fpsStream;
    fpsStream << std::fixed << std::setprecision(1) << fps << " FPS";
    _fpsText.setString(fpsStream.str());
    _fpsText.setFillColor(getFPSColor(fps));
    
    // Stats text
    std::ostringstream statsStream;
    statsStream << std::fixed << std::setprecision(2);
    statsStream << "Frame: " << profiler.getFrameTimeMs() << " ms\n";
    statsStream << "Entities: " << profiler.getEntityCount() << "\n";
    statsStream << "Draw Calls: " << profiler.getDrawCalls() << "\n";
    statsStream << "Memory: " << profiler.getMemoryUsageMB() << " MB";
    _statsText.setString(statsStream.str());
    
    // Network text
    if (_networkMode) {
        auto& netStats = profiler.getNetworkStats();
        std::ostringstream netStream;
        netStream << std::fixed << std::setprecision(1);
        netStream << "Latency: " << netStats.latencyMs << " ms\n";
        netStream << "Jitter: " << netStats.jitterMs << " ms\n";
        netStream << "Sent: " << netStats.packetsSent << " pkts\n";
        netStream << "Recv: " << netStats.packetsReceived << " pkts";
        _networkText.setString(netStream.str());
    }
    
    // Sections text
    const auto& sections = profiler.getAllSections();
    if (!sections.empty() && _mode == OverlayMode::DETAILED) {
        std::ostringstream sectStream;
        sectStream << std::fixed << std::setprecision(2);
        sectStream << "--- Timings ---\n";
        
        // Sort by avg time
        std::vector<std::pair<std::string, ProfileSection>> sorted(sections.begin(), sections.end());
        std::sort(sorted.begin(), sorted.end(),
            [](const auto& a, const auto& b) { return a.second.avgTimeMs > b.second.avgTimeMs; });
        
        int count = 0;
        for (const auto& [name, section] : sorted) {
            if (count++ >= 8) break; // Limit to top 8
            sectStream << name << ": " << section.avgTimeMs << " ms\n";
        }
        _sectionsText.setString(sectStream.str());
    }
}

void ProfilerOverlay::render(sf::RenderWindow& window)
{
    if (!_initialized || _mode == OverlayMode::HIDDEN) {
        return;
    }
    
    // Save current view
    sf::View currentView = window.getView();
    window.setView(window.getDefaultView());
    
    switch (_mode) {
        case OverlayMode::MINIMAL:
            renderMinimal(window);
            break;
        case OverlayMode::COMPACT:
            renderCompact(window);
            break;
        case OverlayMode::FULL:
            renderFull(window);
            break;
        case OverlayMode::DETAILED:
            renderDetailed(window);
            break;
        default:
            break;
    }
    
    // Restore view
    window.setView(currentView);
}

void ProfilerOverlay::renderMinimal(sf::RenderWindow& window)
{
    if (!_fontLoaded) return;
    
    float padding = 5.0f * _scale;
    
    // Background
    sf::FloatRect bounds = _fpsText.getLocalBounds();
    _background.setSize(sf::Vector2f(bounds.width + padding * 2, bounds.height + padding * 2));
    _background.setPosition(_posX, _posY);
    window.draw(_background);
    
    // FPS
    _fpsText.setPosition(_posX + padding, _posY + padding - 4);
    window.draw(_fpsText);
}

void ProfilerOverlay::renderCompact(sf::RenderWindow& window)
{
    if (!_fontLoaded) return;
    
    float padding = 8.0f * _scale;
    float lineHeight = 18.0f * _scale;
    
    // Calculate size
    float width = 160.0f * _scale;
    float height = lineHeight * 5 + padding * 2;
    
    renderBackground(window, width, height);
    
    // FPS
    _fpsText.setPosition(_posX + padding, _posY + padding - 4);
    window.draw(_fpsText);
    
    // Stats
    _statsText.setPosition(_posX + padding, _posY + padding + lineHeight);
    window.draw(_statsText);
}

void ProfilerOverlay::renderFull(sf::RenderWindow& window)
{
    if (!_fontLoaded) return;
    
    float padding = 8.0f * _scale;
    float lineHeight = 18.0f * _scale;
    
    // Calculate size
    float width = std::max(200.0f * _scale, _graphWidth + padding * 2);
    float height = lineHeight * 6 + padding * 2;
    
    if (_networkMode) {
        height += lineHeight * 4;
    }
    
    if (_showGraph) {
        height += _graphHeight + padding;
    }
    
    renderBackground(window, width, height);
    
    float yOffset = _posY + padding;
    
    // FPS
    _fpsText.setPosition(_posX + padding, yOffset - 4);
    window.draw(_fpsText);
    yOffset += lineHeight;
    
    // Stats
    _statsText.setPosition(_posX + padding, yOffset);
    window.draw(_statsText);
    yOffset += lineHeight * 4;
    
    // Network stats
    if (_networkMode) {
        _networkText.setPosition(_posX + padding, yOffset);
        window.draw(_networkText);
        yOffset += lineHeight * 4;
    }
    
    // Graph
    if (_showGraph) {
        renderGraph(window, _posX + padding, yOffset);
    }
}

void ProfilerOverlay::renderDetailed(sf::RenderWindow& window)
{
    if (!_fontLoaded) return;
    
    float padding = 8.0f * _scale;
    float lineHeight = 16.0f * _scale;
    
    // Calculate size
    float width = std::max(220.0f * _scale, _graphWidth + padding * 2);
    float height = lineHeight * 16 + padding * 2;
    
    if (_networkMode) {
        height += lineHeight * 4;
    }
    
    if (_showGraph) {
        height += _graphHeight + padding;
    }
    
    renderBackground(window, width, height);
    
    float yOffset = _posY + padding;
    
    // FPS
    _fpsText.setPosition(_posX + padding, yOffset - 4);
    window.draw(_fpsText);
    yOffset += lineHeight + 4;
    
    // Stats
    _statsText.setPosition(_posX + padding, yOffset);
    window.draw(_statsText);
    yOffset += lineHeight * 4 + 4;
    
    // Sections
    _sectionsText.setPosition(_posX + padding, yOffset);
    window.draw(_sectionsText);
    yOffset += lineHeight * 9;
    
    // Network stats
    if (_networkMode) {
        _networkText.setPosition(_posX + padding, yOffset);
        window.draw(_networkText);
        yOffset += lineHeight * 4;
    }
    
    // Graph
    if (_showGraph) {
        renderGraph(window, _posX + padding, yOffset);
    }
}

void ProfilerOverlay::renderGraph(sf::RenderWindow& window, float x, float y)
{
    auto& profiler = Profiler::getInstance();
    const auto& history = profiler.getFrameTimeHistory();
    
    if (history.empty()) return;
    
    // Graph background
    _graphBackground.setSize(sf::Vector2f(_graphWidth, _graphHeight));
    _graphBackground.setPosition(x, y);
    window.draw(_graphBackground);
    
    // Build vertices
    _graphVertices.clear();
    
    float maxTime = 33.33f; // 30 FPS baseline
    for (double time : history) {
        maxTime = std::max(maxTime, static_cast<float>(time));
    }
    
    float stepX = _graphWidth / static_cast<float>(history.size() - 1);
    
    for (size_t i = 0; i < history.size(); ++i) {
        float xPos = x + i * stepX;
        float normalized = static_cast<float>(history[i]) / maxTime;
        float yPos = y + _graphHeight - (normalized * _graphHeight);
        
        sf::Color color = _graphColor;
        if (history[i] > 33.33) color = _fpsWarning_;
        if (history[i] > 50.0) color = _fpsBad;
        
        _graphVertices.emplace_back(sf::Vector2f(xPos, yPos), color);
    }
    
    if (_graphVertices.size() >= 2) {
        window.draw(_graphVertices.data(), _graphVertices.size(), sf::LineStrip);
    }
    
    // Draw 60 FPS and 30 FPS lines
    float line60 = y + _graphHeight - ((16.67f / maxTime) * _graphHeight);
    float line30 = y + _graphHeight - ((33.33f / maxTime) * _graphHeight);
    
    sf::Vertex line60Verts[2] = {
        sf::Vertex(sf::Vector2f(x, line60), sf::Color(100, 255, 100, 100)),
        sf::Vertex(sf::Vector2f(x + _graphWidth, line60), sf::Color(100, 255, 100, 100))
    };
    sf::Vertex line30Verts[2] = {
        sf::Vertex(sf::Vector2f(x, line30), sf::Color(255, 200, 50, 100)),
        sf::Vertex(sf::Vector2f(x + _graphWidth, line30), sf::Color(255, 200, 50, 100))
    };
    
    window.draw(line60Verts, 2, sf::Lines);
    window.draw(line30Verts, 2, sf::Lines);
}

void ProfilerOverlay::renderBackground(sf::RenderWindow& window, float width, float height)
{
    _background.setSize(sf::Vector2f(width, height));
    _background.setPosition(_posX, _posY);
    
    sf::Color bgColor = _bgColor;
    bgColor.a = static_cast<uint8_t>(_opacity * 255);
    _background.setFillColor(bgColor);
    
    window.draw(_background);
}

bool ProfilerOverlay::handleEvent(const sf::Event& event)
{
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::F3) {
            toggle();
            return true;
        }
        if (event.key.code == sf::Keyboard::F4) {
            cycleMode();
            return true;
        }
    }
    return false;
}

void ProfilerOverlay::setMode(OverlayMode mode)
{
    _mode = mode;
    LOG_DEBUG("PROFILER", "Overlay mode: " + std::to_string(static_cast<int>(mode)));
}

OverlayMode ProfilerOverlay::getMode() const
{
    return _mode;
}

void ProfilerOverlay::cycleMode()
{
    switch (_mode) {
        case OverlayMode::HIDDEN:
            _mode = OverlayMode::MINIMAL;
            break;
        case OverlayMode::MINIMAL:
            _mode = OverlayMode::COMPACT;
            break;
        case OverlayMode::COMPACT:
            _mode = OverlayMode::FULL;
            break;
        case OverlayMode::FULL:
            _mode = OverlayMode::DETAILED;
            break;
        case OverlayMode::DETAILED:
            _mode = OverlayMode::HIDDEN;
            break;
    }
    LOG_DEBUG("PROFILER", "Overlay mode cycled to: " + std::to_string(static_cast<int>(_mode)));
}

void ProfilerOverlay::toggle()
{
    if (_mode == OverlayMode::HIDDEN) {
        _mode = OverlayMode::COMPACT;
    } else {
        _mode = OverlayMode::HIDDEN;
    }
}

void ProfilerOverlay::setPosition(float x, float y)
{
    _posX = x;
    _posY = y;
}

void ProfilerOverlay::setScale(float scale)
{
    _scale = scale;
    if (_fontLoaded) {
        unsigned int fontSize = static_cast<unsigned int>(14 * _scale);
        _fpsText.setCharacterSize(fontSize + 4);
        _statsText.setCharacterSize(fontSize);
        _sectionsText.setCharacterSize(fontSize - 2);
        _networkText.setCharacterSize(fontSize);
    }
}

void ProfilerOverlay::setOpacity(float opacity)
{
    _opacity = std::clamp(opacity, 0.0f, 1.0f);
}

void ProfilerOverlay::setGraphSize(float width, float height)
{
    _graphWidth = width;
    _graphHeight = height;
}

void ProfilerOverlay::setNetworkMode(bool enabled)
{
    _networkMode = enabled;
}

void ProfilerOverlay::setShowGraph(bool show)
{
    _showGraph = show;
}

void ProfilerOverlay::setFPSWarningThreshold(float fps)
{
    _fpsWarning = fps;
}

void ProfilerOverlay::setFPSCriticalThreshold(float fps)
{
    _fpsCritical = fps;
}

sf::Color ProfilerOverlay::getFPSColor(double fps) const
{
    if (fps >= _fpsWarning) {
        return _fpsGood;
    } else if (fps >= _fpsCritical) {
        return _fpsWarning_;
    } else {
        return _fpsBad;
    }
}

std::string ProfilerOverlay::formatTime(double ms) const
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << ms << " ms";
    return oss.str();
}

std::string ProfilerOverlay::formatMemory(size_t bytes) const
{
    std::ostringstream oss;
    if (bytes >= 1024 * 1024 * 1024) {
        oss << std::fixed << std::setprecision(2) << (bytes / (1024.0 * 1024.0 * 1024.0)) << " GB";
    } else if (bytes >= 1024 * 1024) {
        oss << std::fixed << std::setprecision(2) << (bytes / (1024.0 * 1024.0)) << " MB";
    } else if (bytes >= 1024) {
        oss << std::fixed << std::setprecision(2) << (bytes / 1024.0) << " KB";
    } else {
        oss << bytes << " B";
    }
    return oss.str();
}

} // namespace core
} // namespace rtype
