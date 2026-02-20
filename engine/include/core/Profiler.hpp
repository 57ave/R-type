/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** Performance profiler for timing, FPS, memory, and network metrics
*/

#ifndef RTYPE_CORE_PROFILER_HPP
#define RTYPE_CORE_PROFILER_HPP

#include <string>
#include <chrono>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <memory>
#include <deque>
#include <functional>

namespace rtype {
namespace core {

struct ProfileSection {
    std::string name;
    double lastTimeMs = 0.0;
    double avgTimeMs = 0.0;
    double minTimeMs = 999999.0;
    double maxTimeMs = 0.0;
    uint64_t callCount = 0;
    double totalTimeMs = 0.0;
};

struct FrameData {
    double frameTimeMs = 0.0;
    double fps = 0.0;
    uint64_t entityCount = 0;
    uint64_t drawCalls = 0;
    size_t memoryUsageBytes = 0;
};

struct NetworkStats {
    uint64_t packetsSent = 0;
    uint64_t packetsReceived = 0;
    uint64_t bytesSent = 0;
    uint64_t bytesReceived = 0;
    double latencyMs = 0.0;
    double jitterMs = 0.0;
};

// RAII timer — starts on construction, records elapsed time on destruction.
class ScopedProfiler {
public:
    ScopedProfiler(const std::string& sectionName);
    ~ScopedProfiler();

private:
    std::string _sectionName;
    std::chrono::high_resolution_clock::time_point _startTime;
};

// Singleton profiler. Tracks FPS, per-section timings, entity count,
// draw calls, memory usage, and network stats.
class Profiler {
public:
    static Profiler& getInstance();

    Profiler(const Profiler&) = delete;
    Profiler& operator=(const Profiler&) = delete;

    void init();
    void shutdown();
    void reset();

    void beginFrame();
    void endFrame();

    void beginSection(const std::string& name);
    void endSection(const std::string& name);

    void setEntityCount(uint64_t count);
    void addDrawCall();
    void resetDrawCalls();
    void updateMemoryUsage();

    void recordPacketSent(size_t bytes);
    void recordPacketReceived(size_t bytes);
    void updateLatency(double latencyMs);

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

    const std::deque<double>& getFrameTimeHistory() const;
    const std::deque<double>& getFPSHistory() const;

    void setEnabled(bool enabled);
    bool isEnabled() const;
    void setHistorySize(size_t size);

    std::string generateReport() const;
    void logReport() const;

private:
    Profiler();
    ~Profiler();

    void updateHistory();
    size_t estimateMemoryUsage() const;

    bool _enabled = true;
    bool _initialized = false;
    mutable std::mutex _mutex;

    std::chrono::high_resolution_clock::time_point _frameStartTime;
    std::chrono::high_resolution_clock::time_point _lastFrameTime;
    double _currentFrameTimeMs = 0.0;
    double _currentFPS = 0.0;
    double _minFrameTimeMs = 999999.0;
    double _maxFrameTimeMs = 0.0;
    uint64_t _frameCount = 0;
    double _totalFrameTime = 0.0;

    std::unordered_map<std::string, ProfileSection> _sections;
    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> _activeSections;

    FrameData _currentFrame;
    NetworkStats _networkStats;

    std::deque<double> _frameTimeHistory;
    std::deque<double> _fpsHistory;
    size_t _historySize = 120;

    size_t _lastMemoryUsage = 0;
};

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

} // namespace core
} // namespace rtype

#endif // RTYPE_CORE_PROFILER_HPP
