/*
** EPITECH PROJECT, 2025
** R-Type Engine
** File description:
** Lua bindings for the DevConsole system - Implementation
*/

#include <scripting/DevConsoleBindings.hpp>
#include <scripting/LuaState.hpp>
#include <core/Logger.hpp>
#include <iostream>
#include <filesystem>

namespace Scripting {

// Static member definition
rtype::core::DevConsole* DevConsoleBindings::s_Console = nullptr;

void DevConsoleBindings::Register(sol::state& lua, rtype::core::DevConsole* console) {
    if (!console) {
        LOG_ERROR("SCRIPTING", "Cannot register DevConsole bindings: console is null");
        return;
    }
    
    s_Console = console;
    
    // Create Console table in Lua
    auto consoleTable = lua.create_named_table("Console");
    
    // ========================================
    // Output functions
    // ========================================
    
    consoleTable.set_function("print", [](const std::string& message) {
        if (s_Console) {
            s_Console->info(message);
        }
    });
    
    consoleTable.set_function("info", [](const std::string& message) {
        if (s_Console) {
            s_Console->info(message);
        }
    });
    
    consoleTable.set_function("success", [](const std::string& message) {
        if (s_Console) {
            s_Console->success(message);
        }
    });
    
    consoleTable.set_function("warning", [](const std::string& message) {
        if (s_Console) {
            s_Console->warning(message);
        }
    });
    
    consoleTable.set_function("error", [](const std::string& message) {
        if (s_Console) {
            s_Console->error(message);
        }
    });
    
    consoleTable.set_function("clear", []() {
        if (s_Console) {
            s_Console->clear();
        }
    });
    
    // ========================================
    // Command registration
    // ========================================
    
    consoleTable.set_function("register", [&lua](
        const std::string& name,
        const std::string& description,
        const std::string& usage,
        sol::function callback) {
        
        if (!s_Console) {
            LOG_ERROR("SCRIPTING", "Cannot register command '" + name + "': console not initialized");
            return;
        }
        
        // Store the Lua function in the registry to prevent garbage collection
        sol::state_view luaView(lua);
        
        // Create a C++ callback that invokes the Lua function
        auto cppCallback = [callback, name](const std::vector<std::string>& args) -> std::string {
            try {
                // Convert args to Lua table
                sol::state_view luaView = callback.lua_state();
                sol::table argsTable = luaView.create_table();
                for (size_t i = 0; i < args.size(); ++i) {
                    argsTable[i + 1] = args[i];  // Lua tables are 1-indexed
                }
                
                // Call the Lua function
                sol::protected_function_result result = callback(argsTable);
                
                if (!result.valid()) {
                    sol::error err = result;
                    std::string errorMsg = "Lua error in command '" + name + "': " + std::string(err.what());
                    LOG_ERROR("SCRIPTING", errorMsg);
                    return errorMsg;
                }
                
                // Get the return value (string or nil)
                if (result.get_type() == sol::type::string) {
                    return result.get<std::string>();
                }
                return "";
                
            } catch (const std::exception& e) {
                std::string errorMsg = "Exception in Lua command '" + name + "': " + std::string(e.what());
                LOG_ERROR("SCRIPTING", errorMsg);
                return errorMsg;
            }
        };
        
        s_Console->registerCommand(name, description, usage, cppCallback);
        LOG_INFO("SCRIPTING", "Registered Lua command: " + name);
    });
    
    // Simplified register with just name and callback
    consoleTable.set_function("register_simple", [&lua](
        const std::string& name,
        sol::function callback) {
        
        if (!s_Console) return;
        
        auto cppCallback = [callback, name](const std::vector<std::string>& args) -> std::string {
            try {
                sol::state_view luaView = callback.lua_state();
                sol::table argsTable = luaView.create_table();
                for (size_t i = 0; i < args.size(); ++i) {
                    argsTable[i + 1] = args[i];
                }
                
                sol::protected_function_result result = callback(argsTable);
                if (!result.valid()) {
                    sol::error err = result;
                    return "Error: " + std::string(err.what());
                }
                
                if (result.get_type() == sol::type::string) {
                    return result.get<std::string>();
                }
                return "";
            } catch (const std::exception& e) {
                return "Exception: " + std::string(e.what());
            }
        };
        
        s_Console->registerCommand(name, "Lua command: " + name, name, cppCallback);
    });
    
    // Unregister a command
    consoleTable.set_function("unregister", [](const std::string& name) {
        if (s_Console) {
            s_Console->unregisterCommand(name);
            LOG_INFO("SCRIPTING", "Unregistered command: " + name);
        }
    });
    
    // ========================================
    // Command execution
    // ========================================
    
    consoleTable.set_function("execute", [](const std::string& command) {
        if (s_Console) {
            s_Console->execute(command);
        }
    });
    
    // ========================================
    // Console control
    // ========================================
    
    consoleTable.set_function("open", []() {
        if (s_Console) {
            s_Console->open();
        }
    });
    
    consoleTable.set_function("close", []() {
        if (s_Console) {
            s_Console->close();
        }
    });
    
    consoleTable.set_function("toggle", []() {
        if (s_Console) {
            s_Console->toggle();
        }
    });
    
    consoleTable.set_function("is_open", []() -> bool {
        return s_Console ? s_Console->isOpen() : false;
    });
    
    LOG_INFO("SCRIPTING", "DevConsole bindings registered");
}

bool DevConsoleBindings::LoadCommandsFile(sol::state& lua, rtype::core::DevConsole* console, const std::string& path) {
    namespace fs = std::filesystem;
    
    if (!fs::exists(path)) {
        LOG_ERROR("SCRIPTING", "Commands file not found: " + path);
        return false;
    }
    
    // Ensure console bindings are registered
    if (s_Console != console) {
        Register(lua, console);
    }
    
    try {
        auto result = lua.safe_script_file(path);
        
        if (!result.valid()) {
            sol::error err = result;
            LOG_ERROR("SCRIPTING", "Error loading commands file: " + std::string(err.what()));
            return false;
        }
        
        LOG_INFO("SCRIPTING", "Loaded commands from: " + path);
        return true;
        
    } catch (const std::exception& e) {
        LOG_ERROR("SCRIPTING", "Exception loading commands file: " + std::string(e.what()));
        return false;
    }
}

} // namespace Scripting
