#include <scripting/ScriptSystem.hpp>
#include <iostream>

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
        std::cerr << "[ScriptSystem] Error: " << err.what() << std::endl;
    }
}

void ScriptSystem::Shutdown() {
    mUpdateFunction = sol::protected_function();
    std::cout << "[ScriptSystem] Shutdown" << std::endl;
}

void ScriptSystem::LoadScript(const std::string& scriptPath) {
    mScriptPath = scriptPath;
    
    auto& lua = LuaState::Instance().GetState();
    
    if (!LuaState::Instance().LoadScript(scriptPath)) {
        std::cerr << "[ScriptSystem] Failed to load: " << scriptPath << std::endl;
        return;
    }

    // Look for update function in global scope
    sol::optional<sol::protected_function> updateFunc = lua["update"];
    if (updateFunc) {
        mUpdateFunction = updateFunc.value();
        std::cout << "[ScriptSystem] Loaded update function from: " << scriptPath << std::endl;
    } else {
        std::cerr << "[ScriptSystem] No 'update' function found in: " << scriptPath << std::endl;
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
        std::cerr << "[ScriptedSystemLoader] System missing 'signature' field" << std::endl;
        return false;
    }

    // Extract update function
    sol::optional<sol::protected_function> updateOpt = systemTable["update"];
    if (!updateOpt) {
        std::cerr << "[ScriptedSystemLoader] System missing 'update' function" << std::endl;
        return false;
    }

    // Create system
    auto system = std::make_shared<ScriptSystem>();
    system->SetCoordinator(coordinator);

    // TODO: Set system signature based on Lua table
    // This requires coordinator API to set signatures programmatically

    std::cout << "[ScriptedSystemLoader] Registered Lua system" << std::endl;
    return true;
}

} // namespace Scripting
