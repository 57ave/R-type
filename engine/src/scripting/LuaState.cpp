#include <scripting/LuaState.hpp>
#include <iostream>

namespace Scripting {

LuaState::LuaState() {
}

void LuaState::Init() {
    mLua.open_libraries(
        sol::lib::base,
        sol::lib::math,
        sol::lib::string,
        sol::lib::table,
        sol::lib::package,
        sol::lib::os
    );

    // Set default error handler
    mErrorCallback = [](const std::string& error) {
        std::cerr << "[Lua Error] " << error << std::endl;
    };

    std::cout << "[LuaState] Initialized (Lua " << LUA_VERSION_MAJOR << "." 
              << LUA_VERSION_MINOR << ")" << std::endl;
}

void LuaState::Shutdown() {
    mLoadedScripts.clear();
    std::cout << "[LuaState] Shutdown" << std::endl;
}

bool LuaState::LoadScript(const std::string& path) {
    try {
        namespace fs = std::filesystem;
        
        if (!fs::exists(path)) {
            if (mErrorCallback) {
                mErrorCallback("Script file not found: " + path);
            }
            return false;
        }

        auto result = mLua.safe_script_file(path);
        
        if (!result.valid()) {
            sol::error err = result;
            HandleError(err);
            return false;
        }

        // Track script for hot-reload
        ScriptInfo info;
        info.path = fs::absolute(path);
        info.lastModified = fs::last_write_time(info.path);
        mLoadedScripts[path] = info;

        std::cout << "[LuaState] Loaded: " << path << std::endl;
        return true;

    } catch (const std::exception& e) {
        if (mErrorCallback) {
            mErrorCallback("Exception loading script: " + std::string(e.what()));
        }
        return false;
    }
}

bool LuaState::ReloadScript(const std::string& path) {
    std::cout << "[LuaState] Reloading: " << path << std::endl;
    return LoadScript(path);
}

void LuaState::ReloadAllScripts() {
    for (const auto& [path, info] : mLoadedScripts) {
        ReloadScript(path);
    }
}

void LuaState::CheckForChanges() {
    if (!mHotReloadEnabled) return;

    namespace fs = std::filesystem;
    
    for (auto& [path, info] : mLoadedScripts) {
        try {
            if (!fs::exists(info.path)) continue;

            auto currentModified = fs::last_write_time(info.path);
            if (currentModified > info.lastModified) {
                std::cout << "[LuaState] Hot-reload detected: " << path << std::endl;
                ReloadScript(path);
                info.lastModified = currentModified;
            }
        } catch (const std::exception& e) {
            if (mErrorCallback) {
                mErrorCallback("Hot-reload check error: " + std::string(e.what()));
            }
        }
    }
}

void LuaState::HandleError(const sol::error& err) {
    if (mErrorCallback) {
        mErrorCallback(err.what());
    }
}

} // namespace Scripting
