#ifndef GAME_HPP
    #define GAME_HPP
    #include <iostream>
    #include <iostream>
    #include <memory>
    #include <vector>
    #include <algorithm>
    #include <string>
    #include <set>
    #include <functional>

    // Engine includes - Core
    #include <ecs/ECS.hpp>
    #include <ecs/Coordinator.hpp>

    // Engine includes - Rendering
    #include <rendering/sfml/SFMLWindow.hpp>
    #include <rendering/sfml/SFMLRenderer.hpp>
    #include <rendering/sfml/SFMLSprite.hpp>
    #include <rendering/sfml/SFMLTexture.hpp>

    // Engine includes - Input/Time/Audio abstractions
    #include <engine/Input.hpp>
    #include <engine/Keyboard.hpp>
    #include <engine/Clock.hpp>
    #include <engine/Audio.hpp>

    // Network includes
    #include <network/NetworkClient.hpp>
    #include <network/RTypeProtocol.hpp>
    #include <systems/NetworkSystem.hpp>

    // Factories
    #include <factories/EnemyFactory.hpp>
    #include <factories/ProjectileFactory.hpp>
    #include <systems/NetworkSystem.hpp>

    // Systems
    #include <systems/MovementSystem.hpp>
    #include <systems/AnimationSystem.hpp>
    #include <systems/StateMachineAnimationSystem.hpp>
    #include <systems/LifetimeSystem.hpp>
    #include <systems/RenderSystem.hpp>
    #include <systems/MovementPatternSystem.hpp>
    #include <systems/ScrollingBackgroundSystem.hpp>
    #include <systems/BoundarySystem.hpp>
    #include <systems/CollisionSystem.hpp>
    #include <systems/HealthSystem.hpp>

    // Components
    #include <components/Position.hpp>
    #include <components/Velocity.hpp>
    #include <components/Sprite.hpp>
    #include <components/Animation.hpp>
    #include <components/Collider.hpp>
    #include <components/Health.hpp>
    #include <components/Weapon.hpp>
    #include <components/Tag.hpp>
    #include <components/ScrollingBackground.hpp>
    #include <components/MovementPattern.hpp>
    #include <components/Lifetime.hpp>
    #include <components/NetworkId.hpp>

    using namespace rtype::engine::rendering;
    using namespace rtype::engine::rendering::sfml;

    class Game {
        public:
            int Run(int argc, char* argv[]);

            ECS::Entity CreatePlayer(float x, float y, int line = 0);
            ECS::Entity CreateBackground(float x, float y, float windowHeight, bool isFirst);
            ECS::Entity CreateEnemy(float x, float y, MovementPattern::Type pattern);
            ECS::Entity CreateMissile(float x, float y, bool isCharged, int chargeLevel);
            ECS::Entity CreateExplosion(float x, float y);
            ECS::Entity CreateShootEffect(float x, float y, ECS::Entity parent);
            void RegisterEntity(ECS::Entity entity);
            void DestroyEntityDeferred(ECS::Entity entity);
            void ProcessDestroyedEntities();
        private:
            ECS::Coordinator gCoordinator;

            std::vector<ECS::Entity> allEntities;
            std::vector<ECS::Entity> entitiesToDestroy;

            std::unique_ptr<SFMLTexture> backgroundTexture;
            std::unique_ptr<SFMLTexture> playerTexture;
            std::unique_ptr<SFMLTexture> missileTexture;
            std::unique_ptr<SFMLTexture> enemyTexture;
            std::unique_ptr<SFMLTexture> explosionTexture;

            std::vector<SFMLSprite*> allSprites;

            rtype::engine::SoundBuffer shootBuffer;
            rtype::engine::Sound shootSound;
    };

#endif // GAME_HPP