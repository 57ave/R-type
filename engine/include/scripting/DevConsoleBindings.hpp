/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** Lua bindings for the DevConsole system
*/

#pragma once

#include <core/DevConsole.hpp>
#include <sol/sol.hpp>

#include "LuaState.hpp"

namespace Scripting {

/**
 * @brief DevConsoleBindings - Exposes DevConsole to Lua scripts
 *
 * This class provides bindings between the C++ DevConsole and Lua scripts,
 * allowing game logic and commands to be defined in Lua configuration files.
 *
 * Usage in Lua:
 *   -- Register a simple command
 *   Console.register("mycommand", "Description", "mycommand [args]", function(args)
 *       Console.print("Hello from Lua!")
 *       return "Command executed"
 *   end)
 *
 *   -- Print to console
 *   Console.print("Info message")
 *   Console.success("Success!")
 *   Console.warning("Warning!")
 *   Console.error("Error!")
 *
 *   -- Execute a command
 *   Console.execute("help")
 */
class DevConsoleBindings {
public:
    /**
     * @brief Register all DevConsole bindings to the Lua state
     * @param lua Reference to the Lua state
     * @param console Pointer to the DevConsole instance
     */
    static void Register(sol::state& lua, rtype::core::DevConsole* console);

    /**
     * @brief Load and execute a Lua commands configuration file
     * @param lua Reference to the Lua state
     * @param console Pointer to the DevConsole instance
     * @param path Path to the Lua commands file
     * @return true if loading succeeded
     */
    static bool LoadCommandsFile(sol::state& lua, rtype::core::DevConsole* console,
                                 const std::string& path);

private:
    // Store console pointer for lambda callbacks
    static rtype::core::DevConsole* s_Console;
};

}  // namespace Scripting
