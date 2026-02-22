#include <scripting/ScriptSystem.hpp>
#include "core/Logger.hpp"

namespace Scripting {

void ScriptSystem::Init() {
    if (!mScriptPath.empty()) {
        LoadScript(mScriptPath);
    }
}

void ScriptSystem::Update(float dt) {
    if (!mUpdateFunction.valid()) return;

    // Call Lua update function with entities and dt
    auto result = mUpdateFunction(mEntities, dt, mCoordinator);
    
    if (!result.valid()) {
        sol::error err = result;
        LOG_ERROR("SCRIPTSYSTEM", "Error: " + std::string(err.what()));
    }
}

void ScriptSystem::Shutdown() {
    mUpdateFunction = sol::protected_function();
    LOG_INFO("SCRIPTSYSTEM", "Shutdown");
}

void ScriptSystem::LoadScript(const std::string& scriptPath) {
    mScriptPath = scriptPath;
    
    auto& lua = LuaState::Instance().GetState();
    
    if (!LuaState::Instance().LoadScript(scriptPath)) {
        LOG_ERROR("SCRIPTSYSTEM", "Failed to load: " + scriptPath);
        return;
    }

    // Look for update function in global scope
    sol::optional<sol::protected_function> updateFunc = lua["update"];
    if (updateFunc) {
        mUpdateFunction = updateFunc.value();
        LOG_INFO("SCRIPTSYSTEM", "Loaded update function from: " + scriptPath);
    } else {
        LOG_ERROR("SCRIPTSYSTEM", "No 'update' function found in: " + scriptPath);
    }
}

// ScriptedSystemLoader implementation
std::shared_ptr<ScriptSystem> ScriptedSystemLoader::LoadSystem(
    const std::string& scriptPath,
    ECS::Coordinator* coordinator)
{
    auto system = std::make_shared<ScriptSystem>();
    system->SetCoordinator(coordinator);
    system->LoadScript(scriptPath);
    return system;
}

bool ScriptedSystemLoader::RegisterLuaSystem(
    sol::table systemTable,
    ECS::Coordinator* coordinator)
{
    // Extract signature (required components)
    sol::optional<sol::table> signatureOpt = systemTable["signature"];
    if (!signatureOpt) {
        LOG_ERROR("SCRIPTEDSYSTEMLOADER", "System missing 'signature' field");
        return false;
    }

    // Extract update function
    sol::optional<sol::protected_function> updateOpt = systemTable["update"];
    if (!updateOpt) {
        LOG_ERROR("SCRIPTEDSYSTEMLOADER", "System missing 'update' function");
        return false;
    }

    // Create system
    auto system = std::make_shared<ScriptSystem>();
    system->SetCoordinator(coordinator);

    // TODO: Set system signature based on Lua table
    // This requires coordinator API to set signatures programmatically

    LOG_INFO("SCRIPTEDSYSTEMLOADER", "Registered Lua system");
    return true;
}

} // namespace Scripting
