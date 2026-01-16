# R-Type Logger System Documentation

## Overview

The R-Type Logger is a thread-safe, colored console and file logging system with module identification and configurable log levels.

## Log Levels

| Level | Color | Usage |
|-------|-------|-------|
| `DEBUG` | 🔵 Cyan | Detailed information for debugging. Disabled in production. |
| `INFO` | 🟢 Green | General information about application flow. |
| `WARNING` | 🟡 Yellow | Potential issues that don't prevent execution. |
| `ERROR` | 🔴 Red | Critical errors that may cause failures. |
| `OFF` | - | Disable all logging. |

## Log Format

### Console Output (Colored)
```
YYYY-MM-DD HH:MM:SS.mmm [LEVEL][MODULE] Message
```

### File Output
```
YYYY-MM-DD HH:MM:SS.mmm [LEVEL][MODULE] Message
```

## Module Names

Use consistent module names to identify the source of logs:

| Module | Description |
|--------|-------------|
| `LOGGER` | Logger system itself |
| `GENERAL` | Default module for legacy API |
| `ECS` | Entity Component System |
| `NETWORK` | Network/UDP operations |
| `RENDERING` | Rendering system |
| `AUDIO` | Audio system |
| `INPUT` | Input handling |
| `SCRIPTING` | Lua scripting |
| `PHYSICS` | Physics/Collision |
| `GAME` | Game logic |
| `SERVER` | Server-specific |
| `CLIENT` | Client-specific |

## Usage

### Initialization

```cpp
#include "core/Logger.hpp"

int main() {
    // Initialize logger with file output
    auto& logger = rtype::core::Logger::getInstance();
    logger.init(".log", "rtype.log");
    
    // Optional: Configure logger
    logger.setMinLevel(rtype::core::LogLevel::DEBUG);
    logger.setColorEnabled(true);
    
    // ... application code ...
    
    // Shutdown logger
    logger.shutdown();
    return 0;
}
```

### Logging Messages

```cpp
// Using the singleton directly
auto& logger = rtype::core::Logger::getInstance();
logger.debug("ECS", "Entity created with ID: 42");
logger.info("NETWORK", "Connected to server");
logger.warning("AUDIO", "Sound file not found, using fallback");
logger.error("RENDERING", "Failed to load texture");

// Using convenience macros
LOG_DEBUG("ECS", "Entity created with ID: 42");
LOG_INFO("NETWORK", "Connected to server");
LOG_WARNING("AUDIO", "Sound file not found");
LOG_ERROR("RENDERING", "Failed to load texture");
```

### Legacy API (Backward Compatible)

```cpp
// Without module name (uses "GENERAL" as default)
logger.info("Application started");
logger.error("Something went wrong");
```

## Configuration

### Setting Minimum Log Level

```cpp
// Only show WARNING and ERROR in production
logger.setMinLevel(rtype::core::LogLevel::WARNING);

// Show all logs in debug mode
logger.setMinLevel(rtype::core::LogLevel::DEBUG);

// Disable all logging
logger.setMinLevel(rtype::core::LogLevel::OFF);
```

### Controlling Output Destinations

```cpp
// Disable console output (file only)
logger.setConsoleEnabled(false);

// Disable file output (console only)
logger.setFileEnabled(false);

// Disable colors (for terminals that don't support ANSI)
logger.setColorEnabled(false);
```

## File Output

Log files are created in the `.log/` directory with timestamped names:
```
.log/20260112_143022_rtype.log
```

The directory is created automatically if it doesn't exist.

## Thread Safety

The logger is fully thread-safe. All logging operations are protected by a mutex, making it safe to log from multiple threads simultaneously.

## Performance Considerations

- Logs below the minimum level are filtered before formatting (minimal overhead)
- File writes are flushed immediately for reliability
- Module color assignment is cached for consistency

## Example Output

### Console (Colored)
```
2026-01-12 14:30:22.456 [INFO][LOGGER] Logger initialized - File: .log/20260112_143022_rtype.log
2026-01-12 14:30:22.457 [DEBUG][ECS] Coordinator initialized
2026-01-12 14:30:22.458 [INFO][NETWORK] Server listening on port 4242
2026-01-12 14:30:22.460 [WARNING][AUDIO] Audio device not found, audio disabled
2026-01-12 14:30:22.461 [ERROR][RENDERING] Failed to load texture: player.png
```

### File (No Colors)
```
2026-01-12 14:30:22.456 [INFO][LOGGER] Logger initialized - File: .log/20260112_143022_rtype.log
2026-01-12 14:30:22.457 [DEBUG][ECS] Coordinator initialized
2026-01-12 14:30:22.458 [INFO][NETWORK] Server listening on port 4242
2026-01-12 14:30:22.460 [WARNING][AUDIO] Audio device not found, audio disabled
2026-01-12 14:30:22.461 [ERROR][RENDERING] Failed to load texture: player.png
```

## Best Practices

1. **Use specific modules**: Always specify a meaningful module name
2. **Use appropriate levels**: Don't use ERROR for warnings, DEBUG for info
3. **Include context**: Include relevant IDs, values, or states in messages
4. **Initialize early**: Call `init()` at the start of your application
5. **Shutdown cleanly**: Call `shutdown()` before exiting
6. **Production settings**: Set minimum level to INFO or WARNING in release builds

## Adding to CMakeLists.txt

The Logger is part of the `core` library. Make sure to link against it:

```cmake
target_link_libraries(your_target PRIVATE core)
```
