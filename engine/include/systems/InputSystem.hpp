#pragma once

#include <core/Export.hpp>
#include <ecs/Components.hpp>
#include <ecs/Coordinator.hpp>
#include <ecs/System.hpp>
#include <functional>
#include <map>
#include <string>

/**
 * @brief Generic input system using string-based action mapping
 *
 * Instead of hardcoded enums, use configurable action names:
 * - "move_up", "move_down", "move_left", "move_right"
 * - "action1", "action2", "action3" (game defines what these mean)
 * - "pause", "menu", etc.
 *
 * This makes the engine reusable for ANY game genre.
 */
class RTYPE_API InputSystem : public ECS::System {
public:
    using InputHandler = std::function<void(ECS::Entity, const std::string&, float)>;

    explicit InputSystem(ECS::Coordinator* coordinator);
    ~InputSystem() override = default;

    void Init() override;
    void Update(float dt) override;
    void Shutdown() override;

    // Set state for a named action
    void SetActionState(const std::string& action, bool pressed);
    void SetInputHandler(InputHandler handler);

    const char* GetName() const { return "InputSystem"; }
    uint32_t GetSystemVersion() const { return 1; }

private:
    ECS::Coordinator* m_Coordinator;
    std::map<std::string, bool> m_ActionStates;  // action name -> pressed state
    InputHandler m_InputHandler;
};

extern "C" {
ECS::System* CreateSystem(ECS::Coordinator* coordinator);
void DestroySystem(ECS::System* system);
const char* GetSystemName();
uint32_t GetSystemVersion();
}
