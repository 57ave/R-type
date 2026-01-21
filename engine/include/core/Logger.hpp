/*
** EPITECH PROJECT, 2025
** Logger
** File description:
** Enhanced logging system with colored output and file logging
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
// Some platforms or build systems define a global macro named DEBUG which
// conflicts with our LogLevel::DEBUG enum value. Undefine it here so the
// enum declaration can use the symbol name safely.
#ifdef DEBUG
#undef DEBUG
#endif

namespace rtype {
namespace core {

/**
 * @brief Log levels for the logging system
 */
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    OFF = 4
};

/**
 * @brief ANSI color codes for console output
 */
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

/**
 * @brief Enhanced Logger class with colored console output and file logging
 * 
 * Features:
 * - DEBUG/INFO/WARNING/ERROR levels
 * - Colored console output
 * - File output in .log/ folder
 * - Module identification
 * - Thread-safe logging
 * - Configurable minimum log level
 */
class Logger {
public:
    /**
     * @brief Get the singleton instance of Logger
     * @return Reference to the Logger instance
     */
    static Logger& getInstance();

    /**
     * @brief Initialize the logger with file output
     * @param logDirectory Directory for log files (default: ".log")
     * @param logFileName Name of the log file (default: "rtype.log")
     * @return true if initialization successful
     */
    bool init(const std::string& logDirectory = ".log", 
              const std::string& logFileName = "rtype.log");

    /**
     * @brief Shutdown the logger and close file handles
     */
    void shutdown();

    /**
     * @brief Set the minimum log level for output
     * @param level Minimum level to display
     */
    void setMinLevel(LogLevel level);

    /**
     * @brief Get the current minimum log level
     * @return Current minimum log level
     */
    LogLevel getMinLevel() const;

    /**
     * @brief Enable or disable colored console output
     * @param enabled true to enable colors
     */
    void setColorEnabled(bool enabled);

    /**
     * @brief Enable or disable console output
     * @param enabled true to enable console output
     */
    void setConsoleEnabled(bool enabled);

    /**
     * @brief Enable or disable file output
     * @param enabled true to enable file output
     */
    void setFileEnabled(bool enabled);

    /**
     * @brief Log a debug message
     * @param module Module/component name
     * @param message Log message
     */
    void debug(const std::string& module, const std::string& message);

    /**
     * @brief Log an info message
     * @param module Module/component name
     * @param message Log message
     */
    void info(const std::string& module, const std::string& message);

    /**
     * @brief Log a warning message
     * @param module Module/component name
     * @param message Log message
     */
    void warning(const std::string& module, const std::string& message);

    /**
     * @brief Log an error message
     * @param module Module/component name
     * @param message Log message
     */
    void error(const std::string& module, const std::string& message);

    // Legacy API for backward compatibility
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);

    // Delete copy constructor and assignment operator
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

private:
    Logger();
    ~Logger();

    /**
     * @brief Internal logging function
     * @param level Log level
     * @param module Module name
     * @param message Log message
     */
    void log(LogLevel level, const std::string& module, const std::string& message);

    /**
     * @brief Get current timestamp as string
     * @return Formatted timestamp
     */
    std::string getTimestamp() const;

    /**
     * @brief Get level name as string
     * @param level Log level
     * @return Level name
     */
    const char* getLevelName(LogLevel level) const;

    /**
     * @brief Get color code for level
     * @param level Log level
     * @return ANSI color code
     */
    const char* getLevelColor(LogLevel level) const;

    /**
     * @brief Get color for module name
     * @param module Module name
     * @return ANSI color code
     */
    const char* getModuleColor(const std::string& module) const;

    std::ofstream _logFile;
    std::string _logFilePath;
    LogLevel _minLevel;
    bool _colorEnabled;
    bool _consoleEnabled;
    bool _fileEnabled;
    bool _initialized;
    mutable std::mutex _mutex;

    // Module color cache for consistent coloring
    mutable std::unordered_map<std::string, size_t> _moduleColorIndex;
    static constexpr const char* MODULE_COLORS[] = {
        "\033[36m",  // Cyan
        "\033[35m",  // Magenta
        "\033[34m",  // Blue
        "\033[32m",  // Green
        "\033[33m",  // Yellow (but not for warnings)
        "\033[37m"   // White
    };
    static constexpr size_t MODULE_COLOR_COUNT = 6;
};

// Convenience macros for logging with automatic module name
#define LOG_DEBUG(module, msg) rtype::core::Logger::getInstance().debug(module, msg)
#define LOG_INFO(module, msg) rtype::core::Logger::getInstance().info(module, msg)
#define LOG_WARNING(module, msg) rtype::core::Logger::getInstance().warning(module, msg)
#define LOG_ERROR(module, msg) rtype::core::Logger::getInstance().error(module, msg)

} // namespace core
} // namespace rtype

#endif /* !_CORE_LOGGER_ */
