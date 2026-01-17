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
    #include <map>

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

    // Shoot'em Up Module Components
    #include <components/ShootEmUpTags.hpp>
    #include <components/PowerUp.hpp>
    #include <components/AIController.hpp>
    #include <components/Weapon.hpp>
    #include <components/MovementPattern.hpp>
    #include <components/Attachment.hpp>
    #include <components/Effect.hpp>

    // Shoot'em Up Module Factories
    #include <factories/EnemyFactory.hpp>
    #include <factories/ProjectileFactory.hpp>

    // Shoot'em Up Module Systems
    #include <systems/WeaponSystem.hpp>
    #include <systems/MovementPatternSystem.hpp>
    #include <systems/EnemySpawnSystem.hpp>

    // Scripting - Engine generic
    #include <scripting/LuaState.hpp>
    #include <scripting/ComponentBindings.hpp>
    #include <scripting/ScriptSystem.hpp>

    // Scripting - R-Type specific
    #include <scripting/GameScriptBindings.hpp>
    #include <scripting/FactoryBindings.hpp>
    #include <scripting/UIBindings.hpp>

    // UI System
    #include <systems/UISystem.hpp>
    
    // Audio System
    #include <systems/AudioSystem.hpp>

    // Game State Management
    #include "GameStateManager.hpp"

    // Generic Engine Systems
    #include <systems/MovementSystem.hpp>
    #include <systems/AnimationSystem.hpp>
    #include <systems/StateMachineAnimationSystem.hpp>
    #include <systems/LifetimeSystem.hpp>
    #include <systems/RenderSystem.hpp>
    #include <systems/ScrollingBackgroundSystem.hpp>
    #include <systems/BoundarySystem.hpp>
    #include <systems/CollisionSystem.hpp>  // Generic engine CollisionSystem
    #include <systems/HealthSystem.hpp>

    // Generic Engine Components
    #include <components/Position.hpp>
    #include <components/Velocity.hpp>
    #include <components/Sprite.hpp>
    #include <components/Animation.hpp>
    #include <components/Collider.hpp>
    #include <components/Health.hpp>
    #include <components/Tag.hpp>
    #include <components/ScrollingBackground.hpp>
    #include <components/Lifetime.hpp>
    #include <components/NetworkId.hpp>
    #include <components/Boundary.hpp>
    #include <components/AudioSource.hpp>

    // UI Components
    #include <components/UIElement.hpp>
    #include <components/UIText.hpp>
    #include <components/UIButton.hpp>
    #include <components/UISlider.hpp>
    #include <components/UIInputField.hpp>
    #include <components/UIPanel.hpp>
    #include <components/UICheckbox.hpp>
    #include <components/UIDropdown.hpp>

    using namespace eng::engine::rendering;
    using namespace eng::engine::rendering::sfml;

    // Helper function to resolve asset paths from different working directories
    std::string ResolveAssetPath(const std::string& relativePath);

    class Game {
        public:
            int Run(int argc, char* argv[]);

            ECS::Entity CreatePlayer(float x, float y, int line = 0);
            ECS::Entity CreateBackground(float x, float y, float windowHeight, bool isFirst);
            ECS::Entity CreateEnemy(float x, float y, std::string patternType = "straight");
            ECS::Entity CreateMissile(float x, float y, bool isCharged, int chargeLevel);
            ECS::Entity CreateEnemyMissile(float x, float y);  // Enemy projectile
            ECS::Entity CreateExplosion(float x, float y);
            ECS::Entity CreateShootEffect(float x, float y, ECS::Entity parent);
            void RegisterEntity(ECS::Entity entity);
            void DestroyEntityDeferred(ECS::Entity entity);
            void ProcessDestroyedEntities();
            
            // ========================================
            // AUDIO SYSTEM - PUBLIC METHODS
            // ========================================
            
            // Music control
            void PlayMusic(const std::string& musicName, bool loop = true);
            void FadeToMusic(const std::string& musicName, float duration = 1.0f);
            void StopMusic();
            void PauseMusic();
            void ResumeMusic();
            
            // Volume control
            void SetMusicVolume(float volume);
            void SetSFXVolume(float volume);
            float GetMusicVolume() const { return currentMusicVolume; }
            float GetSFXVolume() const { return currentSFXVolume; }
            
            // Stage/Boss management
            void SetCurrentStage(int stage);
            void OnBossSpawned();
            void OnBossDefeated();
            void OnGameOver();
            void OnAllStagesClear();
            
            // Settings persistence
            void SaveUserSettings();
            void LoadUserSettings();
            
            // Difficulty management
            void LoadDifficulty(const std::string& difficulty);

        private:
            ECS::Coordinator gCoordinator;

            std::vector<ECS::Entity> allEntities;
            std::vector<ECS::Entity> entitiesToDestroy;

            bool isNetworkClient = false;  // Track if running as network client

            std::unique_ptr<SFMLTexture> backgroundTexture;
            std::unique_ptr<SFMLTexture> playerTexture;
            std::unique_ptr<SFMLTexture> missileTexture;
            std::unique_ptr<SFMLTexture> enemyTexture;
            std::unique_ptr<SFMLTexture> enemyBulletTexture;  // Texture for enemy bullets
            std::unique_ptr<SFMLTexture> explosionTexture;

            std::vector<SFMLSprite*> allSprites;

            eng::engine::SoundBuffer shootBuffer;
            eng::engine::Sound shootSound;

            // Menu music
            eng::engine::SoundBuffer menuMusicBuffer;
            eng::engine::Sound menuMusic;

            // ========================================
            // AUDIO SYSTEM - PRIVATE MEMBERS
            // ========================================
            
            // Music management
            std::map<std::string, std::unique_ptr<eng::engine::SoundBuffer>> musicBuffers;
            std::unique_ptr<eng::engine::Sound> currentMusicSound;
            std::string currentMusicName;
            
            // Volume settings
            float currentMusicVolume = 70.0f;
            float currentSFXVolume = 80.0f;
            
            // Fade system
            bool isFadingMusic = false;
            float fadeTimer = 0.0f;
            float fadeDuration = 1.0f;
            std::string nextMusicName;
            bool fadeOutComplete = false;
            
            // Stage/Boss tracking
            int currentStage = 1;
            bool isBossFight = false;
            
            // Audio System ECS
            std::shared_ptr<eng::engine::systems::AudioSystem> audioSystem;
            
            // Helper for music fade
            void UpdateMusicFade(float deltaTime);

            // UI System
            std::shared_ptr<UISystem> uiSystem;

            // Scripting systems
            std::shared_ptr<Scripting::ScriptSystem> spawnScriptSystem;
            
            // Window pointer for resolution changes from Lua
            SFMLWindow* m_window = nullptr;
    };

#endif // GAME_HPP