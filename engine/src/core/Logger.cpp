/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** Enhanced logging system implementation
*/

#include "core/Logger.hpp"
#include <filesystem>
#include <ctime>

namespace rtype {
namespace core {

// Static member initialization
constexpr const char* Logger::MODULE_COLORS[];

Logger::Logger()
    : _minLevel(LogLevel::DEBUG)
    , _colorEnabled(true)
    , _consoleEnabled(true)
    , _fileEnabled(true)
    , _initialized(false)
{
}

Logger::~Logger()
{
    shutdown();
}

Logger& Logger::getInstance()
{
    static Logger instance;
    return instance;
}

bool Logger::init(const std::string& logDirectory, const std::string& logFileName)
{
    {
        std::lock_guard<std::mutex> lock(_mutex);

        if (_initialized) {
            return true;
        }

        // Create log directory if it doesn't exist
        try {
            if (!std::filesystem::exists(logDirectory)) {
                std::filesystem::create_directories(logDirectory);
            }
        } catch (const std::filesystem::filesystem_error& e) {
            std::cerr << "[ERROR][LOGGER] Failed to create log directory: " << e.what() << std::endl;
            return false;
        }

        // Generate log file path with timestamp
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = *std::localtime(&time_t);

        std::ostringstream oss;
        oss << logDirectory << "/" 
            << std::put_time(&tm, "%Y%m%d_%H%M%S") << "_" << logFileName;
        _logFilePath = oss.str();

        // Open log file
        _logFile.open(_logFilePath, std::ios::out | std::ios::app);
        if (!_logFile.is_open()) {
            std::cerr << "[ERROR][LOGGER] Failed to open log file: " << _logFilePath << std::endl;
            return false;
        }

        _initialized = true;
    } // Release lock before calling info()

    // Log initialization (outside the lock to avoid deadlock)
    info("LOGGER", "Logger initialized - File: " + _logFilePath);
    return true;
}

void Logger::shutdown()
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (_initialized && _logFile.is_open()) {
        _logFile << getTimestamp() << " [INFO][LOGGER] Logger shutdown" << std::endl;
        _logFile.close();
    }
    _initialized = false;
}

void Logger::setMinLevel(LogLevel level)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _minLevel = level;
}

LogLevel Logger::getMinLevel() const
{
    return _minLevel;
}

void Logger::setColorEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _colorEnabled = enabled;
}

void Logger::setConsoleEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _consoleEnabled = enabled;
}

void Logger::setFileEnabled(bool enabled)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _fileEnabled = enabled;
}

void Logger::debug(const std::string& module, const std::string& message)
{
    log(LogLevel::DEBUG, module, message);
}

void Logger::info(const std::string& module, const std::string& message)
{
    log(LogLevel::INFO, module, message);
}

void Logger::warning(const std::string& module, const std::string& message)
{
    log(LogLevel::WARNING, module, message);
}

void Logger::error(const std::string& module, const std::string& message)
{
    log(LogLevel::ERROR, module, message);
}

// Legacy API implementations
void Logger::debug(const std::string& message)
{
    log(LogLevel::DEBUG, "GENERAL", message);
}

void Logger::info(const std::string& message)
{
    log(LogLevel::INFO, "GENERAL", message);
}

void Logger::warning(const std::string& message)
{
    log(LogLevel::WARNING, "GENERAL", message);
}

void Logger::error(const std::string& message)
{
    log(LogLevel::ERROR, "GENERAL", message);
}

void Logger::log(LogLevel level, const std::string& module, const std::string& message)
{
    if (level < _minLevel) {
        return;
    }

    std::lock_guard<std::mutex> lock(_mutex);

    std::string timestamp = getTimestamp();
    const char* levelName = getLevelName(level);

    // Console output with colors
    if (_consoleEnabled) {
        if (_colorEnabled) {
            const char* levelColor = getLevelColor(level);
            const char* moduleColor = getModuleColor(module);

            std::cout << LogColors::WHITE << timestamp << " "
                      << levelColor << LogColors::BOLD << "[" << levelName << "]" << LogColors::RESET
                      << moduleColor << "[" << module << "]" << LogColors::RESET
                      << " " << message << std::endl;
        } else {
            std::cout << timestamp << " [" << levelName << "][" << module << "] " << message << std::endl;
        }
    }

    // File output (no colors)
    if (_fileEnabled && _logFile.is_open()) {
        _logFile << timestamp << " [" << levelName << "][" << module << "] " << message << std::endl;
        _logFile.flush();
    }
}

std::string Logger::getTimestamp() const
{
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm = *std::localtime(&time_t);

    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

const char* Logger::getLevelName(LogLevel level) const
{
    switch (level) {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        default:                return "UNKNOWN";
    }
}

const char* Logger::getLevelColor(LogLevel level) const
{
    switch (level) {
        case LogLevel::DEBUG:   return LogColors::CYAN;
        case LogLevel::INFO:    return LogColors::GREEN;
        case LogLevel::WARNING: return LogColors::YELLOW;
        case LogLevel::ERROR:   return LogColors::RED;
        default:                return LogColors::WHITE;
    }
}

const char* Logger::getModuleColor(const std::string& module) const
{
    // Get consistent color for each module
    auto it = _moduleColorIndex.find(module);
    if (it == _moduleColorIndex.end()) {
        size_t index = _moduleColorIndex.size() % MODULE_COLOR_COUNT;
        _moduleColorIndex[module] = index;
        return MODULE_COLORS[index];
    }
    return MODULE_COLORS[it->second];
}

} // namespace core
} // namespace rtype



