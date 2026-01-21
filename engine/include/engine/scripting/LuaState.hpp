#pragma once

#define SOL_ALL_SAFETIES_ON 1
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <sol/sol.hpp>
#include <string>
#include <unordered_map>

namespace Scripting {

class LuaState {
public:
    static LuaState& Instance() {
        static LuaState instance;
        return instance;
    }

    void Init();
    void Shutdown();

    sol::state& GetState() { return mLua; }

    // Script loading
    bool LoadScript(const std::string& path);
    bool ReloadScript(const std::string& path);
    void ReloadAllScripts();

    // Hot-reload
    void EnableHotReload(bool enable) { mHotReloadEnabled = enable; }
    void CheckForChanges();

    // Error handling
    using ErrorCallback = std::function<void(const std::string&)>;
    void SetErrorCallback(ErrorCallback callback) { mErrorCallback = callback; }

    // Execute Lua function
    template <typename... Args>
    sol::protected_function_result CallFunction(const std::string& name, Args&&... args) {
        sol::protected_function func = mLua[name];
        if (!func.valid()) {
            if (mErrorCallback) {
                mErrorCallback("Function '" + name + "' not found");
            }
            return sol::protected_function_result();
        }

        auto result = func(std::forward<Args>(args)...);
        if (!result.valid()) {
            sol::error err = result;
            if (mErrorCallback) {
                mErrorCallback("Lua error: " + std::string(err.what()));
            }
        }
        return result;
    }

    // Get global variable
    template <typename T>
    T GetGlobal(const std::string& name) {
        return mLua[name];
    }

    // Set global variable
    template <typename T>
    void SetGlobal(const std::string& name, T value) {
        mLua[name] = value;
    }

private:
    LuaState();
    ~LuaState() = default;
    LuaState(const LuaState&) = delete;
    LuaState& operator=(const LuaState&) = delete;

    sol::state mLua;
    bool mHotReloadEnabled = true;
    ErrorCallback mErrorCallback;

    struct ScriptInfo {
        std::filesystem::path path;
        std::filesystem::file_time_type lastModified;
    };
    std::unordered_map<std::string, ScriptInfo> mLoadedScripts;

    void HandleError(const sol::error& err);
};

}  // namespace Scripting
