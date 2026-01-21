#pragma once

#include <memory>
#include <sol/sol.hpp>
#include <string>

#include "../ecs/Coordinator.hpp"
#include "../ecs/System.hpp"
#include "LuaState.hpp"

namespace Scripting {

// System that executes Lua update functions
class ScriptSystem : public ECS::System {
public:
    void Init() override;
    void Update(float dt) override;
    void Shutdown() override;

    void LoadScript(const std::string& scriptPath);
    void SetCoordinator(ECS::Coordinator* coordinator) { mCoordinator = coordinator; }

private:
    ECS::Coordinator* mCoordinator = nullptr;
    sol::protected_function mUpdateFunction;
    std::string mScriptPath;
};

// Factory for creating scripted systems from Lua
class ScriptedSystemLoader {
public:
    static std::shared_ptr<ScriptSystem> LoadSystem(const std::string& scriptPath,
                                                    ECS::Coordinator* coordinator);

    // Register a Lua table as a system
    static bool RegisterLuaSystem(sol::table systemTable, ECS::Coordinator* coordinator);
};

}  // namespace Scripting
