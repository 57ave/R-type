/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** Lua bindings for Logger and Profiler - Implementation
*/

#include <scripting/CoreBindings.hpp>
#include <algorithm>
#include <cctype>

namespace Scripting {

void CoreBindings::Register(sol::state& lua) {
    RegisterLogger(lua);
    RegisterProfiler(lua);
    
    LOG_SUCCESS("SCRIPTING", "Core bindings (Logger + Profiler) registered");
}

void CoreBindings::RegisterLogger(sol::state& lua) {
    using namespace rtype::core;
    
    // Create Log table
    auto logTable = lua.create_named_table("Log");
    
    // ========================================
    // Logging functions
    // ========================================
    
    logTable.set_function("debug", [](const std::string& category, const std::string& message) {
        LOG_DEBUG(category, message);
    });
    
    logTable.set_function("info", [](const std::string& category, const std::string& message) {
        LOG_INFO(category, message);
    });
    
    logTable.set_function("success", [](const std::string& category, const std::string& message) {
        LOG_SUCCESS(category, message);
    });
    
    logTable.set_function("warning", [](const std::string& category, const std::string& message) {
        LOG_WARNING(category, message);
    });
    
    logTable.set_function("error", [](const std::string& category, const std::string& message) {
        LOG_ERROR(category, message);
    });
    
    // Convenience functions with default category "LUA"
    logTable.set_function("d", [](const std::string& message) {
        LOG_DEBUG("LUA", message);
    });
    
    logTable.set_function("i", [](const std::string& message) {
        LOG_INFO("LUA", message);
    });
    
    logTable.set_function("w", [](const std::string& message) {
        LOG_WARNING("LUA", message);
    });
    
    logTable.set_function("e", [](const std::string& message) {
        LOG_ERROR("LUA", message);
    });
    
    // ========================================
    // Configuration
    // ========================================
    
    logTable.set_function("setLevel", [](const std::string& level) {
        std::string lowerLevel = level;
        std::transform(lowerLevel.begin(), lowerLevel.end(), lowerLevel.begin(), ::tolower);
        
        LogLevel logLevel;
        if (lowerLevel == "debug") logLevel = LogLevel::DEBUG;
        else if (lowerLevel == "info") logLevel = LogLevel::INFO;
        else if (lowerLevel == "warning") logLevel = LogLevel::WARNING;
        else if (lowerLevel == "error") logLevel = LogLevel::ERROR;
        else if (lowerLevel == "off") logLevel = LogLevel::OFF;
        else {
            LOG_WARNING("LUA", "Unknown log level: " + level);
            return;
        }
        
        Logger::getInstance().setMinLevel(logLevel);
        LOG_INFO("LUA", "Log level set to: " + level);
    });
    
    logTable.set_function("getLevel", []() -> std::string {
        auto level = Logger::getInstance().getMinLevel();
        switch (level) {
            case LogLevel::DEBUG: return "debug";
            case LogLevel::INFO: return "info";
            case LogLevel::WARNING: return "warning";
            case LogLevel::ERROR: return "error";
            case LogLevel::OFF: return "off";
            default: return "unknown";
        }
    });
    
    logTable.set_function("enableColors", [](bool enable) {
        Logger::getInstance().setColorEnabled(enable);
    });
    
    logTable.set_function("enableTimestamps", [](bool enable) {
        Logger::getInstance().setTimestampEnabled(enable);
    });
    
    logTable.set_function("setLogFile", [](const std::string& path) {
        Logger::getInstance().setLogFile(path);
        LOG_INFO("LUA", "Log file set to: " + path);
    });
    
    // ========================================
    // Utility
    // ========================================
    
    logTable.set_function("flush", []() {
        Logger::getInstance().flush();
    });
}

void CoreBindings::RegisterProfiler(sol::state& lua) {
    using namespace rtype::core;
    
    // Create Profiler table
    auto profilerTable = lua.create_named_table("Profiler");
    
    // ========================================
    // Section timing
    // ========================================
    
    profilerTable.set_function("beginSection", [](const std::string& name) {
        Profiler::getInstance().beginSection(name);
    });
    
    profilerTable.set_function("endSection", [](const std::string& name) {
        Profiler::getInstance().endSection(name);
    });
    
    // ========================================
    // Frame timing
    // ========================================
    
    profilerTable.set_function("getFPS", []() -> float {
        return Profiler::getInstance().getCurrentFPS();
    });
    
    profilerTable.set_function("getAverageFPS", []() -> float {
        return Profiler::getInstance().getAverageFPS();
    });
    
    profilerTable.set_function("getFrameTime", []() -> float {
        return Profiler::getInstance().getFrameTimeMs();
    });
    
    profilerTable.set_function("getMinFPS", []() -> float {
        return Profiler::getInstance().getMinFPS();
    });
    
    profilerTable.set_function("getMaxFPS", []() -> float {
        return Profiler::getInstance().getMaxFPS();
    });
    
    // ========================================
    // Memory & Stats
    // ========================================
    
    profilerTable.set_function("getMemoryUsage", []() -> float {
        return Profiler::getInstance().getMemoryUsageMB();
    });
    
    profilerTable.set_function("getEntityCount", []() -> size_t {
        return Profiler::getInstance().getEntityCount();
    });
    
    profilerTable.set_function("getDrawCalls", []() -> size_t {
        return Profiler::getInstance().getDrawCalls();
    });
    
    // ========================================
    // Section stats
    // ========================================
    
    profilerTable.set_function("getSectionTime", [](const std::string& name) -> float {
        return Profiler::getInstance().getSectionTimeMs(name);
    });
    
    profilerTable.set_function("getSectionAverage", [](const std::string& name) -> float {
        return Profiler::getInstance().getSectionAverageMs(name);
    });
    
    // ========================================
    // Control
    // ========================================
    
    profilerTable.set_function("enable", []() {
        Profiler::getInstance().setEnabled(true);
        LOG_INFO("LUA", "Profiler enabled");
    });
    
    profilerTable.set_function("disable", []() {
        Profiler::getInstance().setEnabled(false);
        LOG_INFO("LUA", "Profiler disabled");
    });
    
    profilerTable.set_function("isEnabled", []() -> bool {
        return Profiler::getInstance().isEnabled();
    });
    
    profilerTable.set_function("reset", []() {
        Profiler::getInstance().reset();
        LOG_INFO("LUA", "Profiler stats reset");
    });
    
    // ========================================
    // Reporting
    // ========================================
    
    profilerTable.set_function("getReport", []() -> std::string {
        return Profiler::getInstance().generateReport();
    });
    
    profilerTable.set_function("printReport", []() {
        std::string report = Profiler::getInstance().generateReport();
        LOG_INFO("PROFILER", "\n" + report);
    });
    
    // ========================================
    // Network stats (if available)
    // ========================================
    
    profilerTable.set_function("getBytesSent", []() -> size_t {
        return Profiler::getInstance().getBytesSent();
    });
    
    profilerTable.set_function("getBytesReceived", []() -> size_t {
        return Profiler::getInstance().getBytesReceived();
    });
    
    profilerTable.set_function("getPacketsSent", []() -> size_t {
        return Profiler::getInstance().getPacketsSent();
    });
    
    profilerTable.set_function("getPacketsReceived", []() -> size_t {
        return Profiler::getInstance().getPacketsReceived();
    });
    
    // ========================================
    // Convenience: Get all stats as table
    // ========================================
    
    profilerTable.set_function("getStats", [&lua]() -> sol::table {
        auto& profiler = Profiler::getInstance();
        
        sol::table stats = lua.create_table();
        stats["fps"] = profiler.getCurrentFPS();
        stats["avgFps"] = profiler.getAverageFPS();
        stats["minFps"] = profiler.getMinFPS();
        stats["maxFps"] = profiler.getMaxFPS();
        stats["frameTime"] = profiler.getFrameTimeMs();
        stats["memory"] = profiler.getMemoryUsageMB();
        stats["entities"] = profiler.getEntityCount();
        stats["drawCalls"] = profiler.getDrawCalls();
        stats["bytesSent"] = profiler.getBytesSent();
        stats["bytesReceived"] = profiler.getBytesReceived();
        
        return stats;
    });
}

} // namespace Scripting
