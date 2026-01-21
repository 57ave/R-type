/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** Lua bindings for Logger and Profiler
*/

#pragma once

#include <core/Logger.hpp>
#include <core/Profiler.hpp>
#include <sol/sol.hpp>

#include "LuaState.hpp"

namespace Scripting {

/**
 * @brief CoreBindings - Exposes Logger and Profiler to Lua scripts
 *
 * This class provides bindings for core engine utilities:
 * - Logger: For logging messages with different levels
 * - Profiler: For performance monitoring and timing
 *
 * Usage in Lua:
 *   -- Logging
 *   Log.debug("SCRIPT", "Debug message")
 *   Log.info("SCRIPT", "Info message")
 *   Log.warning("SCRIPT", "Warning!")
 *   Log.error("SCRIPT", "Error occurred!")
 *   Log.setLevel("debug")  -- Set minimum log level
 *
 *   -- Profiling
 *   Profiler.beginSection("MySection")
 *   -- ... code to profile ...
 *   Profiler.endSection("MySection")
 *
 *   local fps = Profiler.getFPS()
 *   local frameTime = Profiler.getFrameTime()
 *   local memory = Profiler.getMemoryUsage()
 *
 *   -- Generate report
 *   local report = Profiler.getReport()
 */
class CoreBindings {
public:
    /**
     * @brief Register all core bindings (Logger + Profiler) to Lua
     * @param lua Reference to the Lua state
     */
    static void Register(sol::state& lua);

    /**
     * @brief Register only Logger bindings
     * @param lua Reference to the Lua state
     */
    static void RegisterLogger(sol::state& lua);

    /**
     * @brief Register only Profiler bindings
     * @param lua Reference to the Lua state
     */
    static void RegisterProfiler(sol::state& lua);
};

}  // namespace Scripting
