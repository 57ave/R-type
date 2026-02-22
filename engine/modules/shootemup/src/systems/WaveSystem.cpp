#include <systems/WaveSystem.hpp>
#include "core/Logger.hpp"
#include <algorithm>

namespace ShootEmUp {
namespace Systems {

void WaveSystem::Update(float dt) {
    if (!currentStage_.isActive) return;
    
    // Update wave time
    if (currentStage_.currentWaveIndex < static_cast<int>(currentStage_.waves.size())) {
        auto& wave = currentStage_.waves[currentStage_.currentWaveIndex];
        
        if (wave.isActive) {
            wave.currentTime += dt;
            
            // Process spawns
            ProcessSpawns(dt);
            
            // Check wave completion
            CheckWaveCompletion();
        }
    }
    
    // Handle wave transitions
    if (currentStage_.inTransition) {
        currentStage_.waveTransitionTimer -= dt;
        if (currentStage_.waveTransitionTimer <= 0) {
            currentStage_.inTransition = false;
            TransitionToNextWave();
        }
    }
}

void WaveSystem::LoadStage(int stageNumber) {
    // Clear current stage
    currentStage_ = Components::Stage();
    currentStage_.stageNumber = stageNumber;
    activeEnemies_.clear();
    
    // Stage data must be populated externally (e.g. from Lua at game layer)
    // Use SetStage() to provide the data before calling StartStage().
    LOG_INFO("WAVESYSTEM", "[WaveSystem] Stage " + std::to_string(stageNumber)
              + " slot ready — populate via SetStage() then call StartStage()");
}

void WaveSystem::SetStage(const Components::Stage& stage) {
    currentStage_ = stage;
    activeEnemies_.clear();
    
    LOG_INFO("WAVESYSTEM", "[WaveSystem] Loaded " + currentStage_.stageName
              + " with " + std::to_string(currentStage_.waves.size()) + " waves");
}

void WaveSystem::StartStage() {
    if (currentStage_.waves.empty()) {
        LOG_ERROR("WAVESYSTEM", "[WaveSystem] No waves to start!");
        return;
    }
    
    currentStage_.isActive = true;
    currentStage_.currentWaveIndex = 0;
    currentStage_.totalScore = 0;
    currentStage_.completionTime = 0;
    
    StartWave(0);
    
    LOG_INFO("WAVESYSTEM", "[WaveSystem] Stage " + std::to_string(currentStage_.stageNumber) + " started!");
}

void WaveSystem::EndStage() {
    currentStage_.isActive = false;
    currentStage_.isCompleted = true;
    
    if (stageCompleteCallback_) {
        stageCompleteCallback_(currentStage_.stageNumber, currentStage_.totalScore);
    }
    
    LOG_INFO("WAVESYSTEM", "[WaveSystem] Stage " + std::to_string(currentStage_.stageNumber)
              + " completed! Score: " + std::to_string(currentStage_.totalScore));
}

void WaveSystem::StartWave(int waveIndex) {
    if (waveIndex >= static_cast<int>(currentStage_.waves.size())) {
        EndStage();
        return;
    }
    
    currentStage_.currentWaveIndex = waveIndex;
    auto& wave = currentStage_.waves[waveIndex];
    
    wave.isActive = true;
    wave.currentTime = 0;
    wave.currentSpawnIndex = 0;
    wave.enemiesSpawned = 0;
    wave.enemiesKilled = 0;
    
    // Calculate total enemies
    wave.totalEnemies = 0;
    for (const auto& spawn : wave.spawns) {
        wave.totalEnemies += spawn.count;
    }
    
    LOG_INFO("WAVESYSTEM", "[WaveSystem] Wave " + std::to_string(waveIndex + 1) + " started: "
              + wave.waveName + " (" + std::to_string(wave.totalEnemies) + " enemies)");
    
    // Check for boss wave
    if (wave.isBossWave && !wave.bossType.empty() && bossSpawnCallback_) {
        bossSpawnCallback_(wave.bossType, 1920.0f, 540.0f);
    }
}

void WaveSystem::EndWave() {
    auto& wave = currentStage_.waves[currentStage_.currentWaveIndex];
    wave.isActive = false;
    wave.isCompleted = true;
    
    // Calculate score
    int waveScore = wave.completionScore;
    currentStage_.totalScore += waveScore;
    
    if (waveCompleteCallback_) {
        waveCompleteCallback_(currentStage_.currentWaveIndex + 1, waveScore);
    }
    
    LOG_INFO("WAVESYSTEM", "[WaveSystem] Wave " + std::to_string(currentStage_.currentWaveIndex + 1)
              + " completed! Score: " + std::to_string(waveScore));
    
    // Start transition to next wave
    if (currentStage_.currentWaveIndex + 1 < static_cast<int>(currentStage_.waves.size())) {
        currentStage_.inTransition = true;
        currentStage_.waveTransitionTimer = currentStage_.timeBetweenWaves;
    } else {
        EndStage();
    }
}

bool WaveSystem::IsWaveComplete() const {
    if (currentStage_.currentWaveIndex >= static_cast<int>(currentStage_.waves.size())) {
        return true;
    }
    
    const auto& wave = currentStage_.waves[currentStage_.currentWaveIndex];
    
    // Check time limit
    if (wave.currentTime >= wave.duration) {
        return true;
    }
    
    // Check if all enemies killed (if required)
    if (wave.requireAllKilled) {
        return wave.enemiesKilled >= wave.totalEnemies && 
               wave.currentSpawnIndex >= static_cast<int>(wave.spawns.size());
    }
    
    return false;
}

void WaveSystem::OnEnemyKilled(ECS::Entity enemy, int scoreValue) {
    auto it = std::find(activeEnemies_.begin(), activeEnemies_.end(), enemy);
    if (it != activeEnemies_.end()) {
        activeEnemies_.erase(it);
    }
    
    if (currentStage_.currentWaveIndex < static_cast<int>(currentStage_.waves.size())) {
        currentStage_.waves[currentStage_.currentWaveIndex].enemiesKilled++;
        currentStage_.totalScore += scoreValue;
        gameProgress_.enemiesKilled++;
    }
}

void WaveSystem::OnEnemySpawned(ECS::Entity enemy) {
    activeEnemies_.push_back(enemy);
    
    if (currentStage_.currentWaveIndex < static_cast<int>(currentStage_.waves.size())) {
        currentStage_.waves[currentStage_.currentWaveIndex].enemiesSpawned++;
    }
}

int WaveSystem::GetEnemiesRemaining() const {
    return static_cast<int>(activeEnemies_.size());
}

void WaveSystem::ProcessSpawns(float dt) {
    if (!spawnCallback_) return;
    
    auto& wave = currentStage_.waves[currentStage_.currentWaveIndex];
    
    while (wave.currentSpawnIndex < static_cast<int>(wave.spawns.size())) {
        const auto& spawn = wave.spawns[wave.currentSpawnIndex];
        
        if (wave.currentTime >= spawn.spawnTime) {
            // Spawn the enemy
            ECS::Entity entity = spawnCallback_(spawn);
            if (entity != 0) {
                OnEnemySpawned(entity);
            }
            
            wave.currentSpawnIndex++;
        } else {
            break;  // Future spawn, wait
        }
    }
}

void WaveSystem::CheckWaveCompletion() {
    if (IsWaveComplete()) {
        EndWave();
    }
}

void WaveSystem::TransitionToNextWave() {
    StartWave(currentStage_.currentWaveIndex + 1);
}

} // namespace Systems
} // namespace ShootEmUp
