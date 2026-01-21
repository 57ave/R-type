#include <components/Health.hpp>
#include <components/MovementPattern.hpp>
#include <components/Position.hpp>
#include <components/ShootEmUpTags.hpp>
#include <components/Tag.hpp>
#include <components/Velocity.hpp>
#include <cstdlib>
#include <ctime>
#include <ecs/Coordinator.hpp>
#include <systems/EnemySpawnSystem.hpp>

using namespace ShootEmUp::Components;

EnemySpawnSystem::EnemySpawnSystem() : coordinator_(nullptr) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
}

void EnemySpawnSystem::Init() {}

void EnemySpawnSystem::Shutdown() {}

void EnemySpawnSystem::Update(float dt) {
    if (!coordinator_)
        return;

    spawnTimer_ += dt;

    if (spawnTimer_ >= spawnInterval_) {
        spawnTimer_ = 0.0f;
        SpawnEnemy();
    }
}

void EnemySpawnSystem::SpawnEnemy() {
    if (!coordinator_)
        return;

    // Create enemy entity
    ECS::Entity enemy = coordinator_->CreateEntity();

    // Random Y position
    float spawnY = 100.0f + (std::rand() % static_cast<int>(windowHeight_ - 200.0f));

    // Add Position (spawn at right side of screen)
    Position pos;
    pos.x = windowWidth_ + 50.0f;
    pos.y = spawnY;
    coordinator_->AddComponent<Position>(enemy, pos);

    // Random movement pattern (string-based, configured in Lua)
    MovementPattern pattern;
    const char* patterns[] = {"straight", "sine_wave",     "zigzag",
                              "circular", "diagonal_down", "diagonal_up"};
    int patternIndex = std::rand() % 6;
    pattern.patternType = patterns[patternIndex];
    pattern.speed = 200.0f + (std::rand() % 200);
    pattern.amplitude = 50.0f + (std::rand() % 100);
    pattern.frequency = 1.0f + (std::rand() % 3);
    pattern.startX = pos.x;
    pattern.startY = pos.y;
    coordinator_->AddComponent<MovementPattern>(enemy, pattern);

    // Add Health
    Health health;
    health.current = 1;
    health.max = 1;
    health.destroyOnDeath = true;
    health.deathEffect = "explosion";
    coordinator_->AddComponent<Health>(enemy, health);

    // Add EnemyTag
    EnemyTag tag;
    tag.enemyType = "basic";
    coordinator_->AddComponent<EnemyTag>(enemy, tag);

    // TODO: Add Sprite, Animation, Collider components
}
