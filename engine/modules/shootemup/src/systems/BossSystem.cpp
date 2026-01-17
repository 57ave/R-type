#include <systems/BossSystem.hpp>
#include <components/Sprite.hpp>
#include <cmath>
#include <iostream>
#include <algorithm>

namespace ShootEmUp {
namespace Systems {

// ============================================================================
// BOSS SYSTEM
// ============================================================================

void BossSystem::Update(float dt) {
    if (activeBoss_ == 0) return;
    
    // Check if boss still exists
    if (!coordinator_->HasComponent<Components::Boss>(activeBoss_)) {
        activeBoss_ = 0;
        return;
    }
    
    auto& boss = coordinator_->GetComponent<Components::Boss>(activeBoss_);
    
    // Update entry animation
    if (boss.isEntering) {
        UpdateEntry(activeBoss_, dt);
        return;  // Don't attack during entry
    }
    
    // Update phase based on health
    UpdatePhase(activeBoss_);
    
    // Update movement
    UpdateMovement(activeBoss_, dt);
    
    // Update attacks
    UpdateAttacks(activeBoss_, dt);
    
    // Update hit flash
    if (boss.hitFlashTimer > 0) {
        boss.hitFlashTimer -= dt;
    }
}

void BossSystem::SpawnBoss(ECS::Entity bossEntity) {
    if (!coordinator_->HasComponent<Components::Boss>(bossEntity)) {
        std::cerr << "[BossSystem] Entity " << bossEntity << " has no Boss component!" << std::endl;
        return;
    }
    
    activeBoss_ = bossEntity;
    auto& boss = coordinator_->GetComponent<Components::Boss>(bossEntity);
    
    boss.isEntering = true;
    boss.entryProgress = 0.0f;
    
    std::cout << "[BossSystem] Boss spawned: " << boss.bossName << std::endl;
}

void BossSystem::DestroyBoss(ECS::Entity bossEntity) {
    if (bossEntity != activeBoss_) return;
    
    if (coordinator_->HasComponent<Components::Boss>(bossEntity)) {
        auto& boss = coordinator_->GetComponent<Components::Boss>(bossEntity);
        
        std::cout << "[BossSystem] Boss defeated: " << boss.bossName << std::endl;
        
        if (deathCallback_) {
            deathCallback_(bossEntity, boss.scoreValue);
        }
    }
    
    activeBoss_ = 0;
}

int BossSystem::GetCurrentPhase() const {
    if (activeBoss_ == 0) return 0;
    
    if (coordinator_->HasComponent<Components::Boss>(activeBoss_)) {
        return coordinator_->GetComponent<Components::Boss>(activeBoss_).currentPhase;
    }
    return 0;
}

float BossSystem::GetHealthPercent() const {
    if (activeBoss_ == 0) return 0.0f;
    
    if (coordinator_->HasComponent<Health>(activeBoss_)) {
        auto& health = coordinator_->GetComponent<Health>(activeBoss_);
        return static_cast<float>(health.current) / static_cast<float>(health.max);
    }
    return 0.0f;
}

bool BossSystem::IsInRageMode() const {
    if (activeBoss_ == 0) return false;
    
    if (coordinator_->HasComponent<Components::Boss>(activeBoss_)) {
        return coordinator_->GetComponent<Components::Boss>(activeBoss_).inRageMode;
    }
    return false;
}

bool BossSystem::IsEntering() const {
    if (activeBoss_ == 0) return false;
    
    if (coordinator_->HasComponent<Components::Boss>(activeBoss_)) {
        return coordinator_->GetComponent<Components::Boss>(activeBoss_).isEntering;
    }
    return false;
}

void BossSystem::UpdateEntry(ECS::Entity boss, float dt) {
    auto& bossComp = coordinator_->GetComponent<Components::Boss>(boss);
    auto& pos = coordinator_->GetComponent<Position>(boss);
    
    bossComp.entryProgress += dt / bossComp.entryDuration;
    
    if (bossComp.entryProgress >= 1.0f) {
        bossComp.entryProgress = 1.0f;
        bossComp.isEntering = false;
        pos.x = bossComp.targetX;
        
        std::cout << "[BossSystem] " << bossComp.bossName << " entry complete!" << std::endl;
        return;
    }
    
    // Smooth entry interpolation (ease-out)
    float t = bossComp.entryProgress;
    float eased = 1.0f - (1.0f - t) * (1.0f - t);
    
    // Calculate start X (should come from config)
    float startX = bossComp.targetX + 600.0f;  // Start off-screen
    pos.x = startX + (bossComp.targetX - startX) * eased;
}

void BossSystem::UpdateMovement(ECS::Entity boss, float dt) {
    auto& bossComp = coordinator_->GetComponent<Components::Boss>(boss);
    auto& pos = coordinator_->GetComponent<Position>(boss);
    
    static float movementTime = 0.0f;
    movementTime += dt;
    
    // Apply movement pattern
    if (bossComp.movementPattern == "hover") {
        // Hovering up and down
        float hoverOffset = std::sin(movementTime * bossComp.hoverFrequency) * bossComp.hoverAmplitude;
        
        // Base Y position (center of screen)
        float baseY = 540.0f;
        pos.y = baseY + hoverOffset;
        
        // Slight horizontal movement
        pos.x = bossComp.targetX + std::cos(movementTime * 0.5f) * 30.0f;
    }
    else if (bossComp.movementPattern == "sweep") {
        // Side to side movement
        pos.y = 540.0f + std::sin(movementTime) * bossComp.hoverAmplitude;
        pos.x = bossComp.targetX + std::cos(movementTime * 0.3f) * 150.0f;
    }
    else if (bossComp.movementPattern == "aggressive") {
        // Moves towards player occasionally
        if (playerEntity_ != 0 && coordinator_->HasComponent<Position>(playerEntity_)) {
            auto& playerPos = coordinator_->GetComponent<Position>(playerEntity_);
            
            // Smoothly follow player Y
            float targetY = playerPos.y;
            pos.y += (targetY - pos.y) * dt * 2.0f;
            
            // Stay at target X with slight movement
            pos.x = bossComp.targetX + std::sin(movementTime) * 50.0f;
        }
    }
    else if (bossComp.movementPattern == "stationary") {
        // Boss doesn't move
        pos.x = bossComp.targetX;
    }
    
    // Apply rage mode speed boost
    if (bossComp.inRageMode) {
        // Faster, more erratic movement in rage mode
        pos.y += std::sin(movementTime * 5.0f) * 2.0f;
    }
}

void BossSystem::UpdateAttacks(ECS::Entity boss, float dt) {
    auto& bossComp = coordinator_->GetComponent<Components::Boss>(boss);
    
    // Update attack timer
    bossComp.attackTimer += dt;
    
    // Apply rage mode fire rate boost
    float cooldown = bossComp.attackCooldown;
    if (bossComp.inRageMode) {
        cooldown /= bossComp.rageFireRateMultiplier;
    }
    
    // Time to attack?
    if (bossComp.attackTimer >= cooldown) {
        bossComp.attackTimer = 0.0f;
        
        // Select and execute attack
        std::string attack = SelectNextAttack(bossComp);
        if (!attack.empty()) {
            ExecuteAttack(boss, attack);
        }
    }
}

void BossSystem::UpdatePhase(ECS::Entity boss) {
    auto& bossComp = coordinator_->GetComponent<Components::Boss>(boss);
    
    float healthPercent = GetHealthPercent();
    
    // Check phase transitions
    int newPhase = bossComp.currentPhase;
    
    for (int i = static_cast<int>(bossComp.phaseThresholds.size()) - 1; i >= 0; --i) {
        if (healthPercent <= bossComp.phaseThresholds[i]) {
            newPhase = i + 1;
            break;
        }
    }
    
    // Phase changed?
    if (newPhase != bossComp.currentPhase) {
        int oldPhase = bossComp.currentPhase;
        bossComp.currentPhase = newPhase;
        
        std::cout << "[BossSystem] " << bossComp.bossName << " entered phase " << newPhase << std::endl;
        
        if (phaseChangeCallback_) {
            phaseChangeCallback_(boss, newPhase);
        }
    }
    
    // Check rage mode
    if (!bossComp.inRageMode && healthPercent <= bossComp.rageThreshold) {
        bossComp.inRageMode = true;
        std::cout << "[BossSystem] " << bossComp.bossName << " entered RAGE MODE!" << std::endl;
    }
}

std::string BossSystem::SelectNextAttack(const Components::Boss& boss) {
    // Get attacks for current phase
    if (boss.phasePatterns.empty()) {
        return "";
    }
    
    int phaseIndex = std::min(boss.currentPhase - 1, static_cast<int>(boss.phasePatterns.size()) - 1);
    if (phaseIndex < 0) phaseIndex = 0;
    
    // For simplicity, return the pattern name
    // In a full implementation, this would parse the pattern and return individual attacks
    return boss.phasePatterns[phaseIndex];
}

void BossSystem::ExecuteAttack(ECS::Entity boss, const std::string& attackName) {
    auto& pos = coordinator_->GetComponent<Position>(boss);
    
    if (attackCallback_) {
        attackCallback_(boss, attackName);
    }
    
    // Execute built-in attack types
    if (attackName == "spread" || attackName == "spread_shot") {
        ExecuteSpreadShot(boss, 5, 60.0f);
    }
    else if (attackName == "aimed" || attackName == "aimed_shot") {
        ExecuteAimedShot(boss, 3, 0.2f);
    }
    else if (attackName == "laser_sweep") {
        ExecuteLaserSweep(boss, 90.0f, 3.0f);
    }
    else if (attackName == "bullet_hell") {
        ExecuteBulletHell(boss, 4, 60.0f);
    }
    else if (attackName == "spawn_minions") {
        ExecuteSpawnMinions(boss, "basic", 4);
    }
    else if (attackName == "charge" || attackName == "charge_attack") {
        ExecuteChargeAttack(boss, 600.0f);
    }
}

void BossSystem::ExecuteSpreadShot(ECS::Entity boss, int count, float spreadAngle) {
    if (!projectileSpawnCb_) return;
    
    auto& pos = coordinator_->GetComponent<Position>(boss);
    
    float startAngle = 180.0f - spreadAngle / 2.0f;
    float angleStep = spreadAngle / (count - 1);
    
    for (int i = 0; i < count; ++i) {
        float angle = startAngle + i * angleStep;
        projectileSpawnCb_(pos.x - 50.0f, pos.y + 50.0f, angle, "boss_spread");
    }
    
    std::cout << "[BossSystem] Spread shot: " << count << " projectiles" << std::endl;
}

void BossSystem::ExecuteAimedShot(ECS::Entity boss, int count, float interval) {
    if (!projectileSpawnCb_) return;
    
    auto& pos = coordinator_->GetComponent<Position>(boss);
    float angle = GetAngleToPlayer(pos.x, pos.y);
    
    // For now, fire all at once with slight spread
    for (int i = 0; i < count; ++i) {
        float offsetAngle = angle + (i - count / 2) * 5.0f;
        projectileSpawnCb_(pos.x - 50.0f, pos.y + 50.0f, offsetAngle, "enemy_aimed");
    }
    
    std::cout << "[BossSystem] Aimed shot at angle " << angle << std::endl;
}

void BossSystem::ExecuteLaserSweep(ECS::Entity boss, float sweepAngle, float duration) {
    if (!projectileSpawnCb_) return;
    
    auto& pos = coordinator_->GetComponent<Position>(boss);
    
    // Fire a burst of projectiles in a sweeping pattern
    int projectileCount = 10;
    float startAngle = 180.0f - sweepAngle / 2.0f;
    float angleStep = sweepAngle / projectileCount;
    
    for (int i = 0; i < projectileCount; ++i) {
        float angle = startAngle + i * angleStep;
        projectileSpawnCb_(pos.x - 60.0f, pos.y + 30.0f, angle, "boss_laser_sweep");
    }
    
    std::cout << "[BossSystem] Laser sweep executed" << std::endl;
}

void BossSystem::ExecuteBulletHell(ECS::Entity boss, int arms, float spiralSpeed) {
    if (!projectileSpawnCb_) return;
    
    auto& bossComp = coordinator_->GetComponent<Components::Boss>(boss);
    auto& pos = coordinator_->GetComponent<Position>(boss);
    
    // Spiral bullet pattern
    static float spiralAngle = 0.0f;
    spiralAngle += spiralSpeed * 0.016f;  // Approximate dt
    
    float angleStep = 360.0f / arms;
    
    for (int i = 0; i < arms; ++i) {
        float angle = spiralAngle + i * angleStep;
        projectileSpawnCb_(pos.x, pos.y + 50.0f, angle, "boss_bullet_hell");
    }
}

void BossSystem::ExecuteSpawnMinions(ECS::Entity boss, const std::string& type, int count) {
    if (!minionSpawnCb_) return;
    
    auto& pos = coordinator_->GetComponent<Position>(boss);
    
    for (int i = 0; i < count; ++i) {
        float offsetY = (i - count / 2.0f) * 60.0f;
        minionSpawnCb_(type, pos.x + 50.0f, pos.y + offsetY);
    }
    
    std::cout << "[BossSystem] Spawned " << count << " " << type << " minions" << std::endl;
}

void BossSystem::ExecuteChargeAttack(ECS::Entity boss, float speed) {
    // Set velocity towards player
    if (!coordinator_->HasComponent<Velocity>(boss)) return;
    
    auto& pos = coordinator_->GetComponent<Position>(boss);
    auto& vel = coordinator_->GetComponent<Velocity>(boss);
    
    float angle = GetAngleToPlayer(pos.x, pos.y);
    float radians = angle * 3.14159f / 180.0f;
    
    vel.dx = std::cos(radians) * speed;
    vel.dy = std::sin(radians) * speed;
    
    std::cout << "[BossSystem] Charge attack at speed " << speed << std::endl;
}

float BossSystem::GetAngleToPlayer(float bossX, float bossY) {
    if (playerEntity_ == 0) return 180.0f;  // Default: shoot left
    
    if (!coordinator_->HasComponent<Position>(playerEntity_)) return 180.0f;
    
    auto& playerPos = coordinator_->GetComponent<Position>(playerEntity_);
    
    float dx = playerPos.x - bossX;
    float dy = playerPos.y - bossY;
    
    return std::atan2(dy, dx) * 180.0f / 3.14159f;
}

// ============================================================================
// BOSS PART SYSTEM
// ============================================================================

void BossPartSystem::Update(float dt) {
    UpdatePartPositions(dt);
    UpdatePartAttacks(dt);
    
    // Check for destroyed parts
    for (auto it = bossParts_.begin(); it != bossParts_.end();) {
        ECS::Entity part = it->second;
        
        if (coordinator_->HasComponent<Components::BossPart>(part)) {
            auto& partComp = coordinator_->GetComponent<Components::BossPart>(part);
            
            if (partComp.isDestroyed) {
                // Handle respawn
                if (partComp.respawns) {
                    partComp.respawnTimer -= dt;
                    if (partComp.respawnTimer <= 0) {
                        partComp.isDestroyed = false;
                        partComp.health = partComp.maxHealth;
                        partComp.respawnTimer = partComp.respawnTime;
                    }
                }
                ++it;
            } else {
                // Check health
                if (coordinator_->HasComponent<Health>(part)) {
                    auto& health = coordinator_->GetComponent<Health>(part);
                    if (health.current <= 0) {
                        DestroyPart(part);
                    }
                }
                ++it;
            }
        } else {
            it = bossParts_.erase(it);
        }
    }
}

void BossPartSystem::AttachPart(ECS::Entity boss, ECS::Entity part) {
    bossParts_.push_back({boss, part});
    
    if (coordinator_->HasComponent<Components::BossPart>(part)) {
        coordinator_->GetComponent<Components::BossPart>(part).parentBoss = boss;
    }
}

void BossPartSystem::DestroyPart(ECS::Entity part) {
    for (auto& pair : bossParts_) {
        if (pair.second == part) {
            auto& partComp = coordinator_->GetComponent<Components::BossPart>(part);
            partComp.isDestroyed = true;
            
            if (partDestroyedCb_) {
                partDestroyedCb_(pair.first, part, partComp.partType);
            }
            
            std::cout << "[BossPartSystem] Part destroyed: " << partComp.partType << std::endl;
            break;
        }
    }
}

void BossPartSystem::UpdatePartPositions(float dt) {
    for (auto& pair : bossParts_) {
        ECS::Entity boss = pair.first;
        ECS::Entity part = pair.second;
        
        if (!coordinator_->HasComponent<Position>(boss) ||
            !coordinator_->HasComponent<Position>(part) ||
            !coordinator_->HasComponent<Components::BossPart>(part)) {
            continue;
        }
        
        auto& bossPos = coordinator_->GetComponent<Position>(boss);
        auto& partPos = coordinator_->GetComponent<Position>(part);
        auto& partComp = coordinator_->GetComponent<Components::BossPart>(part);
        
        if (partComp.isDestroyed) continue;
        
        // Update part position relative to boss
        partPos.x = bossPos.x + partComp.offsetX;
        partPos.y = bossPos.y + partComp.offsetY;
    }
}

void BossPartSystem::UpdatePartAttacks(float dt) {
    for (auto& pair : bossParts_) {
        ECS::Entity part = pair.second;
        
        if (!coordinator_->HasComponent<Components::BossPart>(part)) continue;
        
        auto& partComp = coordinator_->GetComponent<Components::BossPart>(part);
        
        if (partComp.isDestroyed || !partComp.canAttack) continue;
        
        // Update fire timer
        partComp.lastFireTime += dt;
        
        if (partComp.lastFireTime >= partComp.fireRate) {
            partComp.lastFireTime = 0;
            
            // Fire! (would need projectile spawn callback)
            // For now just log
            // std::cout << "[BossPartSystem] Part " << partComp.partType << " fires!" << std::endl;
        }
    }
}

} // namespace Systems
} // namespace ShootEmUp
