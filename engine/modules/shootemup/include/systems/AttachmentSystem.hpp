#ifndef RTYPE_ENGINE_SYSTEMS_ATTACHMENTSYSTEM_HPP
#define RTYPE_ENGINE_SYSTEMS_ATTACHMENTSYSTEM_HPP

#include <components/ForcePod.hpp>
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <deque>
#include <ecs/Coordinator.hpp>
#include <ecs/System.hpp>
#include <functional>
#include <vector>

namespace ShootEmUp {
namespace Systems {

/**
 * @brief Force Pod System - Manages the iconic R-Type Force attachment
 *
 * The Force is an indestructible pod that can:
 * - Attach to front or back of ship
 * - Be launched as a weapon
 * - Block enemy bullets
 * - Deal contact damage
 */
class ForcePodSystem : public ECS::System {
public:
    ForcePodSystem() = default;
    explicit ForcePodSystem(ECS::Coordinator* coord) : coordinator_(coord) {}

    void SetCoordinator(ECS::Coordinator* coord) { coordinator_ = coord; }

    void Init() override {}
    void Shutdown() override {}
    void Update(float dt) override;

    // Force pod management
    void CreateForcePod(ECS::Entity owner);
    void DestroyForcePod();
    bool HasForcePod() const { return forcePodEntity_ != 0; }
    ECS::Entity GetForcePod() const { return forcePodEntity_; }

    // Force pod actions
    void AttachToFront();
    void AttachToBack();
    void Detach();
    void Launch();
    void Recall();
    void ToggleAttachment();  // Cycles: detached -> front -> back -> detached

    // Upgrade force
    void UpgradeForce();
    int GetForceLevel() const;

    // Set owner (player)
    void SetOwner(ECS::Entity owner) { ownerEntity_ = owner; }

    // Callbacks
    using ProjectileCallback =
        std::function<ECS::Entity(float x, float y, float angle, const std::string& weaponType)>;
    using CollisionCallback = std::function<void(ECS::Entity forcePod, ECS::Entity other)>;

    void SetProjectileCallback(ProjectileCallback cb) { projectileCb_ = cb; }
    void SetCollisionCallback(CollisionCallback cb) { collisionCb_ = cb; }

    // Force fires when player fires
    void Fire();

private:
    ECS::Coordinator* coordinator_ = nullptr;
    ECS::Entity forcePodEntity_ = 0;
    ECS::Entity ownerEntity_ = 0;

    ProjectileCallback projectileCb_;
    CollisionCallback collisionCb_;

    void UpdateAttached(float dt);
    void UpdateDetached(float dt);
    void UpdateLaunching(float dt);
    void UpdateReturning(float dt);
};

/**
 * @brief Option System - Manages trailing satellites (like Gradius options)
 *
 * Options follow the player with a delay and mirror player attacks.
 */
class OptionSystem : public ECS::System {
public:
    OptionSystem() = default;
    explicit OptionSystem(ECS::Coordinator* coord) : coordinator_(coord) {}

    void SetCoordinator(ECS::Coordinator* coord) { coordinator_ = coord; }

    void Init() override {}
    void Shutdown() override {}
    void Update(float dt) override;

    // Option management
    void AddOption();
    void RemoveOption();
    void RemoveAllOptions();
    int GetOptionCount() const { return static_cast<int>(optionEntities_.size()); }
    int GetMaxOptions() const { return maxOptions_; }

    // Set owner (player)
    void SetOwner(ECS::Entity owner);

    // Formation control
    void SetFormation(const std::string& formation);
    std::string GetFormation() const { return currentFormation_; }
    void CycleFormation();

    // Options fire when player fires
    void Fire();

    // Callbacks
    using ProjectileCallback =
        std::function<ECS::Entity(float x, float y, float angle, const std::string& weaponType)>;
    void SetProjectileCallback(ProjectileCallback cb) { projectileCb_ = cb; }

private:
    ECS::Coordinator* coordinator_ = nullptr;
    ECS::Entity ownerEntity_ = 0;
    std::vector<ECS::Entity> optionEntities_;

    int maxOptions_ = 4;
    std::string currentFormation_ = "trail";

    ProjectileCallback projectileCb_;

    // Position history for trail formation
    std::deque<std::pair<float, float>> positionHistory_;
    int historyMaxSize_ = 120;  // 2 seconds at 60fps

    void UpdateTrailFormation(float dt);
    void UpdateSpreadFormation(float dt);
    void UpdateRotateFormation(float dt);
    void UpdateFixedFormation(float dt);

    void RecordPosition(float x, float y);
    std::pair<float, float> GetHistoryPosition(int framesBack);
};

/**
 * @brief Shield System - Manages temporary protective barriers
 */
class ShieldSystem : public ECS::System {
public:
    ShieldSystem() = default;
    explicit ShieldSystem(ECS::Coordinator* coord) : coordinator_(coord) {}

    void SetCoordinator(ECS::Coordinator* coord) { coordinator_ = coord; }

    void Init() override {}
    void Shutdown() override {}
    void Update(float dt) override;

    // Shield management
    void ActivateShield(ECS::Entity owner, float duration, int hitPoints);
    void DeactivateShield(ECS::Entity owner);
    bool HasShield(ECS::Entity owner) const;
    int GetShieldHits(ECS::Entity owner) const;

    // Shield takes damage
    void OnShieldHit(ECS::Entity owner, int damage);

    // Callbacks
    using ShieldBreakCallback = std::function<void(ECS::Entity owner)>;
    void SetShieldBreakCallback(ShieldBreakCallback cb) { shieldBreakCb_ = cb; }

private:
    ECS::Coordinator* coordinator_ = nullptr;
    std::vector<ECS::Entity> activeShields_;

    ShieldBreakCallback shieldBreakCb_;
};

}  // namespace Systems
}  // namespace ShootEmUp

#endif  // RTYPE_ENGINE_SYSTEMS_ATTACHMENTSYSTEM_HPP
