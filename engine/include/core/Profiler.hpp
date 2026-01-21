/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** Performance profiler for timing, FPS, memory, and system metrics
*/

#ifndef RTYPE_CORE_PROFILER_HPP
#define RTYPE_CORE_PROFILER_HPP

#include <chrono>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace rtype {
namespace core {

/**
 * @brief Stores timing data for a profiled section
 */
struct ProfileSection {
    std::string name;
    double lastTimeMs = 0.0;
    double avgTimeMs = 0.0;
    double minTimeMs = 999999.0;
    double maxTimeMs = 0.0;
    uint64_t callCount = 0;
    double totalTimeMs = 0.0;
};

/**
 * @brief Frame timing data
 */
struct FrameData {
    double frameTimeMs = 0.0;
    double fps = 0.0;
    uint64_t entityCount = 0;
    uint64_t drawCalls = 0;
    size_t memoryUsageBytes = 0;
};

/**
 * @brief Network statistics
 */
struct NetworkStats {
    uint64_t packetsSent = 0;
    uint64_t packetsReceived = 0;
    uint64_t bytesSent = 0;
    uint64_t bytesReceived = 0;
    double latencyMs = 0.0;
    double jitterMs = 0.0;
};

/**
 * @brief RAII-based scope timer for automatic profiling
 */
class ScopedProfiler {
public:
    ScopedProfiler(const std::string& sectionName);
    ~ScopedProfiler();

private:
    std::string _sectionName;
    std::chrono::high_resolution_clock::time_point _startTime;
};

/**
 * @brief Main profiler class - singleton pattern
 *
 * Features:
 * - FPS tracking with history
 * - Section timing (systems, rendering, etc.)
 * - Memory usage estimation
 * - Network latency tracking
 * - Frame time history for graphs
 */
class Profiler {
public:
    static Profiler& getInstance();

    // Prevent copying
    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    // ========================================
    // Lifecycle
    // ========================================
    void init();
    void shutdown();
    void reset();

    // ========================================
    // Frame management
    // ========================================
    void beginFrame();
    void endFrame();

    // ========================================
    // Section profiling
    // ========================================
    void beginSection(const std::string& name);
    void endSection(const std::string& name);

    // ========================================
    // Metrics update
    // ========================================
    void setEntityCount(uint64_t count);
    void addDrawCall();
    void resetDrawCalls();
    void updateMemoryUsage();

    // ========================================
    // Network metrics
    // ========================================
    void recordPacketSent(size_t bytes);
    void recordPacketReceived(size_t bytes);
    void updateLatency(double latencyMs);

    // ========================================
    // Getters
    // ========================================
    double getCurrentFPS() const;
    double getAverageFPS() const;
    double getFrameTimeMs() const;
    double getMinFrameTimeMs() const;
    double getMaxFrameTimeMs() const;
    uint64_t getEntityCount() const;
    uint64_t getDrawCalls() const;
    size_t getMemoryUsageMB() const;

    const ProfileSection* getSection(const std::string& name) const;
    const std::unordered_map<std::string, ProfileSection>& getAllSections() const;
    const NetworkStats& getNetworkStats() const;

    // Frame time history for graphs (last N frames)
    const std::deque<double>& getFrameTimeHistory() const;
    const std::deque<double>& getFPSHistory() const;

    // ========================================
    // Configuration
    // ========================================
    void setEnabled(bool enabled);
    bool isEnabled() const;
    void setHistorySize(size_t size);

    // ========================================
    // Reporting
    // ========================================
    std::string generateReport() const;
    void logReport() const;

private:
    Profiler();
    ~Profiler();

    void updateHistory();
    size_t estimateMemoryUsage() const;

private:
    bool _enabled = true;
    bool _initialized = false;
    mutable std::mutex _mutex;

    // Frame timing
    std::chrono::high_resolution_clock::time_point _frameStartTime;
    std::chrono::high_resolution_clock::time_point _lastFrameTime;
    double _currentFrameTimeMs = 0.0;
    double _currentFPS = 0.0;
    double _minFrameTimeMs = 999999.0;
    double _maxFrameTimeMs = 0.0;
    uint64_t _frameCount = 0;
    double _totalFrameTime = 0.0;

    // Section timing
    std::unordered_map<std::string, ProfileSection> _sections;
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> _activeSections;

    // Current frame data
    FrameData _currentFrame;

    // Network stats
    NetworkStats _networkStats;

    // History for graphs
    std::deque<double> _frameTimeHistory;
    std::deque<double> _fpsHistory;
    size_t _historySize = 120;  // 2 seconds at 60 FPS

    // Memory tracking
    size_t _lastMemoryUsage = 0;
};

// ========================================
// Profiling Macros
// ========================================

#ifdef RTYPE_PROFILING_ENABLED
#define PROFILE_FUNCTION() rtype::core::ScopedProfiler _profiler##__LINE__(__FUNCTION__)
#define PROFILE_SCOPE(name) rtype::core::ScopedProfiler _profiler##__LINE__(name)
#define PROFILE_BEGIN(name) rtype::core::Profiler::getInstance().beginSection(name)
#define PROFILE_END(name) rtype::core::Profiler::getInstance().endSection(name)
#define PROFILE_FRAME_BEGIN() rtype::core::Profiler::getInstance().beginFrame()
#define PROFILE_FRAME_END() rtype::core::Profiler::getInstance().endFrame()
#else
#define PROFILE_FUNCTION()
#define PROFILE_SCOPE(name)
#define PROFILE_BEGIN(name)
#define PROFILE_END(name)
#define PROFILE_FRAME_BEGIN()
#define PROFILE_FRAME_END()
#endif

}  // namespace core
}  // namespace rtype

#endif  // RTYPE_CORE_PROFILER_HPP
