/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** Performance profiler implementation
*/

#include "core/Profiler.hpp"
#include "core/Logger.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

#ifdef __APPLE__
    #include <mach/mach.h>
#elif defined(__linux__)
    #include <fstream>
    #include <unistd.h>
#elif defined(_WIN32)
    #include <windows.h>
    #include <psapi.h>
#endif

namespace rtype {
namespace core {

// ========================================
// ScopedProfiler Implementation
// ========================================

ScopedProfiler::ScopedProfiler(const std::string& sectionName)
    : _sectionName(sectionName)
    , _startTime(std::chrono::high_resolution_clock::now())
{
    Profiler::getInstance().beginSection(_sectionName);
}

ScopedProfiler::~ScopedProfiler()
{
    Profiler::getInstance().endSection(_sectionName);
}

// ========================================
// Profiler Implementation
// ========================================

Profiler::Profiler()
    : _enabled(true)
    , _initialized(false)
{
}

Profiler::~Profiler()
{
    shutdown();
}

Profiler& Profiler::getInstance()
{
    static Profiler instance;
    return instance;
}

void Profiler::init()
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (_initialized) {
        return;
    }
    
    _frameCount = 0;
    _totalFrameTime = 0.0;
    _minFrameTimeMs = 999999.0;
    _maxFrameTimeMs = 0.0;
    _currentFPS = 0.0;
    _currentFrameTimeMs = 0.0;
    
    _sections.clear();
    _activeSections.clear();
    _frameTimeHistory.clear();
    _fpsHistory.clear();
    
    _networkStats = NetworkStats{};
    _currentFrame = FrameData{};
    
    _lastFrameTime = std::chrono::high_resolution_clock::now();
    _initialized = true;
    
    LOG_INFO("PROFILER", "Profiler initialized");
}

void Profiler::shutdown()
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (!_initialized) {
        return;
    }
    
    _initialized = false;
    LOG_INFO("PROFILER", "Profiler shutdown - Total frames: " + std::to_string(_frameCount));
}

void Profiler::reset()
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    _frameCount = 0;
    _totalFrameTime = 0.0;
    _minFrameTimeMs = 999999.0;
    _maxFrameTimeMs = 0.0;
    
    for (auto& [name, section] : _sections) {
        section.callCount = 0;
        section.totalTimeMs = 0.0;
        section.minTimeMs = 999999.0;
        section.maxTimeMs = 0.0;
        section.avgTimeMs = 0.0;
    }
    
    _frameTimeHistory.clear();
    _fpsHistory.clear();
    _networkStats = NetworkStats{};
}

void Profiler::beginFrame()
{
    if (!_enabled) return;
    
    std::lock_guard<std::mutex> lock(_mutex);
    _frameStartTime = std::chrono::high_resolution_clock::now();
    _currentFrame.drawCalls = 0;
}

void Profiler::endFrame()
{
    if (!_enabled) return;
    
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto now = std::chrono::high_resolution_clock::now();
    
    // Calculate frame time
    auto frameDuration = std::chrono::duration<double, std::milli>(now - _frameStartTime);
    _currentFrameTimeMs = frameDuration.count();
    
    // Calculate time since last frame (for FPS)
    auto timeSinceLastFrame = std::chrono::duration<double>(now - _lastFrameTime);
    _lastFrameTime = now;
    
    // Update FPS
    if (timeSinceLastFrame.count() > 0) {
        _currentFPS = 1.0 / timeSinceLastFrame.count();
    }
    
    // Update stats
    _frameCount++;
    _totalFrameTime += _currentFrameTimeMs;
    _minFrameTimeMs = std::min(_minFrameTimeMs, _currentFrameTimeMs);
    _maxFrameTimeMs = std::max(_maxFrameTimeMs, _currentFrameTimeMs);
    
    // Update current frame data
    _currentFrame.frameTimeMs = _currentFrameTimeMs;
    _currentFrame.fps = _currentFPS;
    
    // Update history
    updateHistory();
}

void Profiler::updateHistory()
{
    // Frame time history
    _frameTimeHistory.push_back(_currentFrameTimeMs);
    if (_frameTimeHistory.size() > _historySize) {
        _frameTimeHistory.pop_front();
    }
    
    // FPS history
    _fpsHistory.push_back(_currentFPS);
    if (_fpsHistory.size() > _historySize) {
        _fpsHistory.pop_front();
    }
}

void Profiler::beginSection(const std::string& name)
{
    if (!_enabled) return;
    
    std::lock_guard<std::mutex> lock(_mutex);
    _activeSections[name] = std::chrono::high_resolution_clock::now();
}

void Profiler::endSection(const std::string& name)
{
    if (!_enabled) return;
    
    auto endTime = std::chrono::high_resolution_clock::now();
    
    std::lock_guard<std::mutex> lock(_mutex);
    
    auto it = _activeSections.find(name);
    if (it == _activeSections.end()) {
        LOG_WARNING("PROFILER", "endSection called without matching beginSection: " + name);
        return;
    }
    
    auto duration = std::chrono::duration<double, std::milli>(endTime - it->second);
    double timeMs = duration.count();
    
    // Update or create section
    auto& section = _sections[name];
    section.name = name;
    section.lastTimeMs = timeMs;
    section.callCount++;
    section.totalTimeMs += timeMs;
    section.avgTimeMs = section.totalTimeMs / section.callCount;
    section.minTimeMs = std::min(section.minTimeMs, timeMs);
    section.maxTimeMs = std::max(section.maxTimeMs, timeMs);
    
    _activeSections.erase(it);
}

void Profiler::setEntityCount(uint64_t count)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _currentFrame.entityCount = count;
}

void Profiler::addDrawCall()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _currentFrame.drawCalls++;
}

void Profiler::resetDrawCalls()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _currentFrame.drawCalls = 0;
}

void Profiler::updateMemoryUsage()
{
    std::lock_guard<std::mutex> lock(_mutex);
    _lastMemoryUsage = estimateMemoryUsage();
    _currentFrame.memoryUsageBytes = _lastMemoryUsage;
}

size_t Profiler::estimateMemoryUsage() const
{
#ifdef __APPLE__
    struct mach_task_basic_info info;
    mach_msg_type_number_t size = MACH_TASK_BASIC_INFO_COUNT;
    if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &size) == KERN_SUCCESS) {
        return info.resident_size;
    }
    return 0;
#elif defined(__linux__)
    std::ifstream statm("/proc/self/statm");
    if (statm.is_open()) {
        size_t size, resident;
        statm >> size >> resident;
        return resident * sysconf(_SC_PAGESIZE);
    }
    return 0;
#elif defined(_WIN32)
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
#else
    return 0;
#endif
}

void Profiler::recordPacketSent(size_t bytes)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _networkStats.packetsSent++;
    _networkStats.bytesSent += bytes;
}

void Profiler::recordPacketReceived(size_t bytes)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _networkStats.packetsReceived++;
    _networkStats.bytesReceived += bytes;
}

void Profiler::updateLatency(double latencyMs)
{
    std::lock_guard<std::mutex> lock(_mutex);
    // Simple moving average for jitter
    double oldLatency = _networkStats.latencyMs;
    _networkStats.latencyMs = latencyMs;
    _networkStats.jitterMs = std::abs(latencyMs - oldLatency) * 0.1 + _networkStats.jitterMs * 0.9;
}

double Profiler::getCurrentFPS() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _currentFPS;
}

double Profiler::getAverageFPS() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (_totalFrameTime > 0 && _frameCount > 0) {
        return 1000.0 * _frameCount / _totalFrameTime;
    }
    return 0.0;
}

double Profiler::getFrameTimeMs() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _currentFrameTimeMs;
}

double Profiler::getMinFrameTimeMs() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _minFrameTimeMs;
}

double Profiler::getMaxFrameTimeMs() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _maxFrameTimeMs;
}

uint64_t Profiler::getEntityCount() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _currentFrame.entityCount;
}

uint64_t Profiler::getDrawCalls() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _currentFrame.drawCalls;
}

size_t Profiler::getMemoryUsageMB() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _lastMemoryUsage / (1024 * 1024);
}

const ProfileSection* Profiler::getSection(const std::string& name) const
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _sections.find(name);
    if (it != _sections.end()) {
        return &it->second;
    }
    return nullptr;
}

const std::unordered_map<std::string, ProfileSection>& Profiler::getAllSections() const
{
    return _sections;
}

const NetworkStats& Profiler::getNetworkStats() const
{
    return _networkStats;
}

const std::deque<double>& Profiler::getFrameTimeHistory() const
{
    return _frameTimeHistory;
}

const std::deque<double>& Profiler::getFPSHistory() const
{
    return _fpsHistory;
}

void Profiler::setEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _enabled = enabled;
}

bool Profiler::isEnabled() const
{
    return _enabled;
}

void Profiler::setHistorySize(size_t size)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _historySize = size;
    while (_frameTimeHistory.size() > _historySize) {
        _frameTimeHistory.pop_front();
    }
    while (_fpsHistory.size() > _historySize) {
        _fpsHistory.pop_front();
    }
}

std::string Profiler::generateReport() const
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    std::ostringstream report;
    report << std::fixed << std::setprecision(2);
    
    report << "========== PROFILER REPORT ==========\n";
    report << "Total Frames: " << _frameCount << "\n";
    report << "Current FPS: " << _currentFPS << "\n";
    report << "Average FPS: " << ((_totalFrameTime > 0) ? (1000.0 * _frameCount / _totalFrameTime) : 0.0) << "\n";
    report << "Frame Time: " << _currentFrameTimeMs << " ms (min: " << _minFrameTimeMs << ", max: " << _maxFrameTimeMs << ")\n";
    report << "Entities: " << _currentFrame.entityCount << "\n";
    report << "Draw Calls: " << _currentFrame.drawCalls << "\n";
    report << "Memory: " << (_lastMemoryUsage / (1024 * 1024)) << " MB\n";
    
    if (!_sections.empty()) {
        report << "\n--- Section Timings ---\n";
        
        // Sort sections by total time
        std::vector<std::pair<std::string, ProfileSection>> sortedSections(_sections.begin(), _sections.end());
        std::sort(sortedSections.begin(), sortedSections.end(),
            [](const auto& a, const auto& b) { return a.second.totalTimeMs > b.second.totalTimeMs; });
        
        for (const auto& [name, section] : sortedSections) {
            report << "  " << name << ": "
                   << section.avgTimeMs << " ms avg, "
                   << section.callCount << " calls, "
                   << section.totalTimeMs << " ms total\n";
        }
    }
    
    if (_networkStats.packetsSent > 0 || _networkStats.packetsReceived > 0) {
        report << "\n--- Network Stats ---\n";
        report << "  Packets Sent: " << _networkStats.packetsSent << " (" << (_networkStats.bytesSent / 1024) << " KB)\n";
        report << "  Packets Received: " << _networkStats.packetsReceived << " (" << (_networkStats.bytesReceived / 1024) << " KB)\n";
        report << "  Latency: " << _networkStats.latencyMs << " ms (jitter: " << _networkStats.jitterMs << " ms)\n";
    }
    
    report << "======================================\n";
    
    return report.str();
}

void Profiler::logReport() const
{
    LOG_INFO("PROFILER", generateReport());
}

} // namespace core
} // namespace rtype
