#ifndef RTYPE_ENGINE_SYSTEMS_WAVESYSTEM_HPP
#define RTYPE_ENGINE_SYSTEMS_WAVESYSTEM_HPP

#include <components/Wave.hpp>
#include <ecs/Coordinator.hpp>
#include <ecs/System.hpp>
#include <functional>
#include <memory>

namespace ShootEmUp {
namespace Systems {

/**
 * @brief Wave System - Manages enemy waves and stage progression
 */
class WaveSystem : public ECS::System {
public:
    WaveSystem() = default;
    explicit WaveSystem(ECS::Coordinator* coord) : coordinator_(coord) {}

    void SetCoordinator(ECS::Coordinator* coord) { coordinator_ = coord; }

    void Init() override {}
    void Shutdown() override {}
    void Update(float dt) override;

    // Stage management
    void LoadStage(int stageNumber);
    void StartStage();
    void EndStage();
    bool IsStageComplete() const { return currentStage_.isCompleted; }
    bool IsStageFailed() const { return currentStage_.isFailed; }

    // Wave management
    void StartWave(int waveIndex);
    void EndWave();
    bool IsWaveComplete() const;
    int GetCurrentWaveNumber() const { return currentStage_.currentWaveIndex + 1; }
    int GetTotalWaves() const { return static_cast<int>(currentStage_.waves.size()); }

    // Enemy tracking
    void OnEnemyKilled(ECS::Entity enemy, int scoreValue);
    void OnEnemySpawned(ECS::Entity enemy);
    int GetEnemiesRemaining() const;

    // Callbacks
    using SpawnCallback = std::function<ECS::Entity(const Components::EnemySpawnInfo&)>;
    using BossSpawnCallback =
        std::function<ECS::Entity(const std::string& bossType, float x, float y)>;
    using WaveCompleteCallback = std::function<void(int waveNumber, int score)>;
    using StageCompleteCallback = std::function<void(int stageNumber, int totalScore)>;

    void SetSpawnCallback(SpawnCallback cb) { spawnCallback_ = cb; }
    void SetBossSpawnCallback(BossSpawnCallback cb) { bossSpawnCallback_ = cb; }
    void SetWaveCompleteCallback(WaveCompleteCallback cb) { waveCompleteCallback_ = cb; }
    void SetStageCompleteCallback(StageCompleteCallback cb) { stageCompleteCallback_ = cb; }

    // Progress
    Components::GameProgress& GetProgress() { return gameProgress_; }
    const Components::Stage& GetCurrentStage() const { return currentStage_; }

    // Predefined stages
    void CreateStage1();
    void CreateStage2();
    void CreateStage3();

private:
    ECS::Coordinator* coordinator_ = nullptr;

    Components::Stage currentStage_;
    Components::GameProgress gameProgress_;

    std::vector<ECS::Entity> activeEnemies_;

    SpawnCallback spawnCallback_;
    BossSpawnCallback bossSpawnCallback_;
    WaveCompleteCallback waveCompleteCallback_;
    StageCompleteCallback stageCompleteCallback_;

    void ProcessSpawns(float dt);
    void CheckWaveCompletion();
    void TransitionToNextWave();
};

}  // namespace Systems
}  // namespace ShootEmUp

#endif  // RTYPE_ENGINE_SYSTEMS_WAVESYSTEM_HPP
