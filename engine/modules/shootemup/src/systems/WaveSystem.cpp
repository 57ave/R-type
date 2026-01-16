#include <systems/WaveSystem.hpp>
#include <iostream>
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
    
    // Stage data should be loaded from Lua
    // For now, use built-in stages
    switch (stageNumber) {
        case 1: CreateStage1(); break;
        case 2: CreateStage2(); break;
        case 3: CreateStage3(); break;
        default:
            std::cerr << "[WaveSystem] Unknown stage: " << stageNumber << std::endl;
            return;
    }
    
    std::cout << "[WaveSystem] Loaded " << currentStage_.stageName 
              << " with " << currentStage_.waves.size() << " waves" << std::endl;
}

void WaveSystem::StartStage() {
    if (currentStage_.waves.empty()) {
        std::cerr << "[WaveSystem] No waves to start!" << std::endl;
        return;
    }
    
    currentStage_.isActive = true;
    currentStage_.currentWaveIndex = 0;
    currentStage_.totalScore = 0;
    currentStage_.completionTime = 0;
    
    StartWave(0);
    
    std::cout << "[WaveSystem] Stage " << currentStage_.stageNumber << " started!" << std::endl;
}

void WaveSystem::EndStage() {
    currentStage_.isActive = false;
    currentStage_.isCompleted = true;
    
    if (stageCompleteCallback_) {
        stageCompleteCallback_(currentStage_.stageNumber, currentStage_.totalScore);
    }
    
    std::cout << "[WaveSystem] Stage " << currentStage_.stageNumber 
              << " completed! Score: " << currentStage_.totalScore << std::endl;
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
    
    std::cout << "[WaveSystem] Wave " << (waveIndex + 1) << " started: " 
              << wave.waveName << " (" << wave.totalEnemies << " enemies)" << std::endl;
    
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
    
    std::cout << "[WaveSystem] Wave " << (currentStage_.currentWaveIndex + 1) 
              << " completed! Score: " << waveScore << std::endl;
    
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

// ============================================================================
// PREDEFINED STAGES
// These should eventually be loaded from Lua, but provide defaults
// ============================================================================

void WaveSystem::CreateStage1() {
    currentStage_.stageName = "Space Colony";
    currentStage_.backgroundMusic = "stage1_bgm";
    currentStage_.timeBetweenWaves = 3.0f;
    
    // Wave 1: Introduction
    Components::Wave wave1;
    wave1.waveNumber = 1;
    wave1.waveName = "First Contact";
    wave1.duration = 25.0f;
    wave1.requireAllKilled = true;
    wave1.completionScore = 500;
    
    wave1.spawns = {
        {"basic", 1.0f, 1920, 200, "straight", 1, 0.3f, "single"},
        {"basic", 1.5f, 1920, 400, "straight", 1, 0.3f, "single"},
        {"basic", 2.0f, 1920, 600, "straight", 1, 0.3f, "single"},
        {"zigzag", 4.0f, 1920, 300, "zigzag", 1, 0.3f, "single"},
        {"zigzag", 5.0f, 1920, 500, "zigzag", 1, 0.3f, "single"},
        {"basic", 8.0f, 1920, 200, "straight", 5, 0.3f, "line"},
        {"sinewave", 12.0f, 1920, 400, "sinewave", 1, 0.3f, "single"},
        {"shooter", 16.0f, 1920, 500, "straight", 1, 0.3f, "single"},
    };
    wave1.totalEnemies = 12;
    
    currentStage_.waves.push_back(wave1);
    
    // Wave 2: Pressure
    Components::Wave wave2;
    wave2.waveNumber = 2;
    wave2.waveName = "Pressure";
    wave2.duration = 30.0f;
    wave2.requireAllKilled = true;
    wave2.completionScore = 750;
    
    wave2.spawns = {
        {"basic", 0.0f, 1920, 150, "straight", 4, 0.3f, "line"},
        {"shooter", 3.0f, 1920, 300, "straight", 1, 0.3f, "single"},
        {"shooter", 3.5f, 1920, 600, "straight", 1, 0.3f, "single"},
        {"zigzag", 6.0f, 1920, 200, "zigzag", 3, 0.5f, "single"},
        {"kamikaze", 10.0f, 1920, 540, "chase", 1, 0.3f, "single"},
        {"sinewave", 12.0f, 1920, 300, "sinewave", 2, 1.0f, "single"},
        {"spreader", 18.0f, 1920, 400, "sinewave", 1, 0.3f, "single"},
        {"basic", 22.0f, 1920, 200, "straight", 8, 0.2f, "line"},
    };
    wave2.totalEnemies = 22;
    
    currentStage_.waves.push_back(wave2);
    
    // Wave 3: Elite
    Components::Wave wave3;
    wave3.waveNumber = 3;
    wave3.waveName = "Elite Squad";
    wave3.duration = 35.0f;
    wave3.requireAllKilled = true;
    wave3.completionScore = 1000;
    
    wave3.spawns = {
        {"elite_fighter", 0.0f, 1920, 300, "evasive", 1, 0.3f, "single"},
        {"elite_fighter", 2.0f, 1920, 600, "evasive", 1, 0.3f, "single"},
        {"armored", 5.0f, 1920, 450, "straight", 1, 0.3f, "single"},
        {"turret", 10.0f, 1920, 150, "stationary", 1, 0.3f, "single"},
        {"turret", 10.0f, 1920, 850, "stationary", 1, 0.3f, "single"},
        {"formation_leader", 18.0f, 1920, 450, "hover", 1, 0.3f, "single"},
        {"shielded", 25.0f, 1920, 450, "zigzag", 1, 0.3f, "single"},
    };
    wave3.totalEnemies = 8;
    
    currentStage_.waves.push_back(wave3);
    
    // Boss Wave
    Components::Wave bossWave;
    bossWave.waveNumber = 4;
    bossWave.waveName = "BOSS: Dobkeratops";
    bossWave.duration = 120.0f;
    bossWave.isBossWave = true;
    bossWave.bossType = "stage1_boss";
    bossWave.requireAllKilled = true;
    bossWave.completionScore = 5000;
    bossWave.totalEnemies = 1;
    
    currentStage_.waves.push_back(bossWave);
}

void WaveSystem::CreateStage2() {
    currentStage_.stageName = "Asteroid Belt";
    currentStage_.backgroundMusic = "stage2_bgm";
    currentStage_.timeBetweenWaves = 3.0f;
    currentStage_.difficultyLevel = 2;
    
    // Similar structure with harder enemies
    Components::Wave wave1;
    wave1.waveNumber = 1;
    wave1.waveName = "Asteroid Field";
    wave1.duration = 30.0f;
    wave1.requireAllKilled = true;
    wave1.completionScore = 750;
    wave1.enemyHealthMultiplier = 1.2f;
    wave1.enemySpeedMultiplier = 1.1f;
    
    wave1.spawns = {
        {"sinewave", 1.0f, 1920, 300, "sinewave", 2, 0.5f, "single"},
        {"kamikaze", 4.0f, 1920, 400, "chase", 1, 0.3f, "single"},
        {"basic", 6.0f, 1920, 200, "straight", 4, 0.25f, "line"},
        {"turret", 10.0f, 1920, 180, "stationary", 2, 5.0f, "single"},
        {"shooter", 14.0f, 1920, 450, "sinewave", 1, 0.3f, "single"},
        {"armored", 20.0f, 1920, 500, "straight", 1, 0.3f, "single"},
    };
    wave1.totalEnemies = 12;
    
    currentStage_.waves.push_back(wave1);
    
    // More waves would be added here...
    
    // Boss Wave
    Components::Wave bossWave;
    bossWave.waveNumber = 4;
    bossWave.waveName = "BOSS: Gomander";
    bossWave.duration = 150.0f;
    bossWave.isBossWave = true;
    bossWave.bossType = "stage2_boss";
    bossWave.requireAllKilled = true;
    bossWave.completionScore = 7500;
    bossWave.totalEnemies = 1;
    
    currentStage_.waves.push_back(bossWave);
}

void WaveSystem::CreateStage3() {
    currentStage_.stageName = "Warship Assault";
    currentStage_.backgroundMusic = "stage3_bgm";
    currentStage_.timeBetweenWaves = 3.0f;
    currentStage_.difficultyLevel = 3;
    
    // Wave with lots of turrets
    Components::Wave wave1;
    wave1.waveNumber = 1;
    wave1.waveName = "Outer Defenses";
    wave1.duration = 40.0f;
    wave1.requireAllKilled = true;
    wave1.completionScore = 1000;
    wave1.enemyHealthMultiplier = 1.5f;
    wave1.enemyFireRateMultiplier = 1.2f;
    
    wave1.spawns = {
        {"turret", 0.0f, 1920, 150, "stationary", 5, 200.0f, "line"},
        {"elite_fighter", 5.0f, 1920, 300, "evasive", 2, 2.0f, "single"},
        {"shooter", 10.0f, 1920, 250, "straight", 4, 0.5f, "single"},
        {"armored", 15.0f, 1920, 500, "straight", 2, 1.0f, "single"},
        {"kamikaze", 25.0f, 1920, 400, "chase", 5, 0.3f, "single"},
        {"shielded", 32.0f, 1920, 500, "straight", 1, 0.3f, "single"},
    };
    wave1.totalEnemies = 20;
    
    currentStage_.waves.push_back(wave1);
    
    // Boss Wave
    Components::Wave bossWave;
    bossWave.waveNumber = 4;
    bossWave.waveName = "BOSS: Battleship Green";
    bossWave.duration = 180.0f;
    bossWave.isBossWave = true;
    bossWave.bossType = "stage3_boss";
    bossWave.requireAllKilled = true;
    bossWave.completionScore = 10000;
    bossWave.totalEnemies = 1;
    
    currentStage_.waves.push_back(bossWave);
}

} // namespace Systems
} // namespace ShootEmUp
