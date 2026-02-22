#ifndef SHOOTEMUP_SYSTEMS_ENEMYSPAWNSYSTEM_HPP
#define SHOOTEMUP_SYSTEMS_ENEMYSPAWNSYSTEM_HPP

#include <ecs/System.hpp>

namespace ECS {
    class Coordinator;
}

class EnemySpawnSystem : public ECS::System {
    public:
        EnemySpawnSystem();
        ~EnemySpawnSystem() override = default;

        void Init() override;
        void Update(float dt) override;
        void Shutdown() override;

        void SetCoordinator(ECS::Coordinator* coordinator) { coordinator_ = coordinator; }
        void SetWindowSize(float width, float height) {
            windowWidth_ = width;
            windowHeight_ = height;
        }

    private:
        ECS::Coordinator* coordinator_;
        float spawnTimer_ = 0.0f;
        float spawnInterval_ = 2.0f;
        float windowWidth_ = 1920.0f;
        float windowHeight_ = 1080.0f;

        void SpawnEnemy();
};

#endif // SHOOTEMUP_SYSTEMS_ENEMYSPAWNSYSTEM_HPP
