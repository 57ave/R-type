/*
** EPITECH PROJECT, 2025
** Logger
** File description:
** Logging system with colored console output and file logging
*/

#ifndef _CORE_LOGGER_
#define _CORE_LOGGER_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <memory>
#include <unordered_map>

namespace rtype {
namespace core {

enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    OFF = 4
};

namespace LogColors {
    constexpr const char* RESET = "\033[0m";
    constexpr const char* RED = "\033[31m";
    constexpr const char* GREEN = "\033[32m";
    constexpr const char* YELLOW = "\033[33m";
    constexpr const char* BLUE = "\033[34m";
    constexpr const char* MAGENTA = "\033[35m";
    constexpr const char* CYAN = "\033[36m";
    constexpr const char* WHITE = "\033[37m";
    constexpr const char* BOLD = "\033[1m";
}

// Thread-safe singleton logger. Supports DEBUG/INFO/WARNING/ERROR levels,
// colored console output, and optional file output under .log/.
class Logger {
public:
    static Logger& getInstance();

    bool init(const std::string& logDirectory = ".log",
              const std::string& logFileName = "rtype.log");
    void shutdown();

    void setMinLevel(LogLevel level);
    LogLevel getMinLevel() const;
    void setColorEnabled(bool enabled);
    void setConsoleEnabled(bool enabled);
    void setFileEnabled(bool enabled);

    void debug(const std::string& module, const std::string& message);
    void info(const std::string& module, const std::string& message);
    void warning(const std::string& module, const std::string& message);
    void error(const std::string& module, const std::string& message);

    // Legacy API
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();
    ~Logger();

    void log(LogLevel level, const std::string& module, const std::string& message);
    std::string getTimestamp() const;
    const char* getLevelName(LogLevel level) const;
    const char* getLevelColor(LogLevel level) const;
    const char* getModuleColor(const std::string& module) const;

    std::ofstream _logFile;
    std::string _logFilePath;
    LogLevel _minLevel;
    bool _colorEnabled;
    bool _consoleEnabled;
    bool _fileEnabled;
    bool _initialized;
    mutable std::mutex _mutex;

    mutable std::unordered_map<std::string, size_t> _moduleColorIndex;
    static constexpr const char* MODULE_COLORS[] = {
        "\033[36m",  // Cyan
        "\033[35m",  // Magenta
        "\033[34m",  // Blue
        "\033[32m",  // Green
        "\033[33m",  // Yellow
        "\033[37m"   // White
    };
    static constexpr size_t MODULE_COLOR_COUNT = 6;
};

#define LOG_DEBUG(module, msg) rtype::core::Logger::getInstance().debug(module, msg)
#define LOG_INFO(module, msg) rtype::core::Logger::getInstance().info(module, msg)
#define LOG_WARNING(module, msg) rtype::core::Logger::getInstance().warning(module, msg)
#define LOG_ERROR(module, msg) rtype::core::Logger::getInstance().error(module, msg)

} // namespace core
} // namespace rtype

#endif /* !_CORE_LOGGER_ */
