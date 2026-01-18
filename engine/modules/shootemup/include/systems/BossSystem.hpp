#ifndef RTYPE_ENGINE_SYSTEMS_BOSSSYSTEM_HPP
#define RTYPE_ENGINE_SYSTEMS_BOSSSYSTEM_HPP

#include <ecs/System.hpp>
#include <ecs/Coordinator.hpp>
#include <components/Boss.hpp>
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Health.hpp>
#include <functional>
#include <memory>

namespace ShootEmUp {
namespace Systems {

/**
 * @brief Boss System - Manages boss behavior, phases, and attacks
 * 
 * This is an ABSTRACT system that handles boss logic without knowing
 * specific boss types. All boss data comes from configuration (Lua).
 */
class BossSystem : public ECS::System {
public:
    BossSystem() = default;
    explicit BossSystem(ECS::Coordinator* coord) : coordinator_(coord) {}
    
    void SetCoordinator(ECS::Coordinator* coord) { coordinator_ = coord; }
    
    void Init() override {}
    void Shutdown() override {}
    void Update(float dt) override;
    
    // Boss lifecycle
    void SpawnBoss(ECS::Entity bossEntity);
    void DestroyBoss(ECS::Entity bossEntity);
    bool IsBossActive() const { return activeBoss_ != 0; }
    ECS::Entity GetActiveBoss() const { return activeBoss_; }
    
    // Boss state
    int GetCurrentPhase() const;
    float GetHealthPercent() const;
    bool IsInRageMode() const;
    bool IsEntering() const;
    
    // Callbacks
    using AttackCallback = std::function<void(ECS::Entity boss, const std::string& attackName)>;
    using PhaseChangeCallback = std::function<void(ECS::Entity boss, int newPhase)>;
    using DeathCallback = std::function<void(ECS::Entity boss, int score)>;
    using ProjectileSpawnCallback = std::function<ECS::Entity(float x, float y, float angle, 
                                                               const std::string& weaponType)>;
    using MinionSpawnCallback = std::function<ECS::Entity(const std::string& enemyType, float x, float y)>;
    
    void SetAttackCallback(AttackCallback cb) { attackCallback_ = cb; }
    void SetPhaseChangeCallback(PhaseChangeCallback cb) { phaseChangeCallback_ = cb; }
    void SetDeathCallback(DeathCallback cb) { deathCallback_ = cb; }
    void SetProjectileSpawnCallback(ProjectileSpawnCallback cb) { projectileSpawnCb_ = cb; }
    void SetMinionSpawnCallback(MinionSpawnCallback cb) { minionSpawnCb_ = cb; }
    
    // Player reference for targeting
    void SetPlayerEntity(ECS::Entity player) { playerEntity_ = player; }
    
private:
    ECS::Coordinator* coordinator_ = nullptr;
    ECS::Entity activeBoss_ = 0;
    ECS::Entity playerEntity_ = 0;
    
    // Callbacks
    AttackCallback attackCallback_;
    PhaseChangeCallback phaseChangeCallback_;
    DeathCallback deathCallback_;
    ProjectileSpawnCallback projectileSpawnCb_;
    MinionSpawnCallback minionSpawnCb_;
    
    // Update functions
    void UpdateEntry(ECS::Entity boss, float dt);
    void UpdateMovement(ECS::Entity boss, float dt);
    void UpdateAttacks(ECS::Entity boss, float dt);
    void UpdatePhase(ECS::Entity boss);
    void UpdateWeakPoints(ECS::Entity boss, float dt);
    
    // Attack execution
    void ExecuteAttack(ECS::Entity boss, const std::string& attackName);
    void ExecuteSpreadShot(ECS::Entity boss, int count, float spreadAngle);
    void ExecuteAimedShot(ECS::Entity boss, int count, float interval);
    void ExecuteLaserSweep(ECS::Entity boss, float angle, float duration);
    void ExecuteBulletHell(ECS::Entity boss, int arms, float spiralSpeed);
    void ExecuteSpawnMinions(ECS::Entity boss, const std::string& type, int count);
    void ExecuteChargeAttack(ECS::Entity boss, float speed);
    
    // Helper functions
    float GetAngleToPlayer(float bossX, float bossY);
    std::string SelectNextAttack(const Components::Boss& boss);
};

/**
 * @brief Boss Part System - Manages destroyable boss parts
 */
class BossPartSystem : public ECS::System {
public:
    BossPartSystem() = default;
    explicit BossPartSystem(ECS::Coordinator* coord) : coordinator_(coord) {}
    
    void SetCoordinator(ECS::Coordinator* coord) { coordinator_ = coord; }
    
    void Init() override {}
    void Shutdown() override {}
    void Update(float dt) override;
    
    // Part management
    void AttachPart(ECS::Entity boss, ECS::Entity part);
    void DestroyPart(ECS::Entity part);
    
    // Callbacks
    using PartDestroyedCallback = std::function<void(ECS::Entity boss, ECS::Entity part, 
                                                      const std::string& partType)>;
    void SetPartDestroyedCallback(PartDestroyedCallback cb) { partDestroyedCb_ = cb; }
    
private:
    ECS::Coordinator* coordinator_ = nullptr;
    std::vector<std::pair<ECS::Entity, ECS::Entity>> bossParts_;  // (boss, part) pairs
    PartDestroyedCallback partDestroyedCb_;
    
    void UpdatePartPositions(float dt);
    void UpdatePartAttacks(float dt);
};

} // namespace Systems
} // namespace ShootEmUp

#endif // RTYPE_ENGINE_SYSTEMS_BOSSSYSTEM_HPP
