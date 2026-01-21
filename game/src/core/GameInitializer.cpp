#include "core/GameInitializer.hpp"
#include <core/Logger.hpp>

// Include all necessary components and systems
#include <ecs/Components.hpp>
#include <components/Position.hpp>
#include <components/Velocity.hpp>
#include <components/Sprite.hpp>
#include <components/Animation.hpp>
#include <components/Collider.hpp>
#include <components/Health.hpp>
#include <components/Boundary.hpp>
#include <components/Tag.hpp>
#include <components/ScrollingBackground.hpp>
#include <components/Lifetime.hpp>
#include <components/Damage.hpp>
#include <components/NetworkId.hpp>
#include <components/AudioSource.hpp>

// ShootEmUp components
#include <components/Weapon.hpp>
#include <components/ShootEmUpTags.hpp>
#include <components/MovementPattern.hpp>
#include <components/Effect.hpp>
#include <components/Score.hpp>

// UI components
#include <components/UIElement.hpp>
#include <components/UIText.hpp>
#include <components/UIButton.hpp>
#include <components/UISlider.hpp>
#include <components/UIInputField.hpp>
#include <components/UIPanel.hpp>
#include <components/UICheckbox.hpp>
#include <components/UIDropdown.hpp>

// Systems
#include <systems/MovementSystem.hpp>
#include <systems/AnimationSystem.hpp>
#include <systems/StateMachineAnimationSystem.hpp>
#include <systems/LifetimeSystem.hpp>
#include <systems/ScrollingBackgroundSystem.hpp>
#include <systems/BoundarySystem.hpp>
#include <systems/CollisionSystem.hpp>
#include <systems/HealthSystem.hpp>
#include <systems/RenderSystem.hpp>
#include <systems/UISystem.hpp>
#include <systems/MovementPatternSystem.hpp>

// Scripting
#include <scripting/ComponentBindings.hpp>
#include "network/NetworkBindings.hpp"

namespace RType::Core {

bool GameInitializer::InitializeECS(ECS::Coordinator& coordinator, 
                                   eng::engine::rendering::sfml::SFMLRenderer& renderer,
                                   ::Scripting::LuaState& luaState) {
    auto& logger = rtype::core::Logger::getInstance();
    logger.setMinLevel(rtype::core::LogLevel::DEBUG);  // Enable DEBUG logs
    logger.info("GameInitializer", "Starting ECS initialization...");
    
    // Initialize ECS Coordinator
    logger.info("GameInitializer", "About to call coordinator.Init()...");
    coordinator.Init();
    logger.info("GameInitializer", "ECS Coordinator initialized");
    
    // Register all components
    RegisterComponents(coordinator);
    
    // Register all systems
    if (!RegisterSystems(coordinator, renderer)) {
        logger.error("GameInitializer", "Failed to register systems");
        return false;
    }
    
    // Setup Lua bindings
    if (!SetupLuaBindings(luaState, coordinator)) {
        logger.error("GameInitializer", "Failed to setup Lua bindings");
        return false;
    }
    
    logger.info("GameInitializer", "ECS initialization completed successfully");
    return true;
}

void GameInitializer::RegisterComponents(ECS::Coordinator& coordinator) {
    auto& logger = rtype::core::Logger::getInstance();
    logger.info("GameInitializer", "Registering components...");
    
    try {
        // Core engine components
        coordinator.RegisterComponent<Position>();
        coordinator.RegisterComponent<Velocity>();
        coordinator.RegisterComponent<Sprite>();
        coordinator.RegisterComponent<Animation>();
        coordinator.RegisterComponent<StateMachineAnimation>();
        coordinator.RegisterComponent<Collider>();
        coordinator.RegisterComponent<Health>();
        coordinator.RegisterComponent<Boundary>();
        coordinator.RegisterComponent<Tag>();
        coordinator.RegisterComponent<ScrollingBackground>();
        coordinator.RegisterComponent<Lifetime>();
        coordinator.RegisterComponent<Damage>();
        coordinator.RegisterComponent<ChargeAnimation>();
        coordinator.RegisterComponent<NetworkId>();
        
        // Audio components
        coordinator.RegisterComponent<AudioSource>();
        coordinator.RegisterComponent<SoundEffect>();
        
        // ShootEmUp specific components
        coordinator.RegisterComponent<ShootEmUp::Components::Weapon>();
        coordinator.RegisterComponent<ShootEmUp::Components::PlayerTag>();
        coordinator.RegisterComponent<ShootEmUp::Components::EnemyTag>();
        coordinator.RegisterComponent<ShootEmUp::Components::ProjectileTag>();
        coordinator.RegisterComponent<ShootEmUp::Components::MovementPattern>();
        coordinator.RegisterComponent<ShootEmUp::Components::Effect>();
        coordinator.RegisterComponent<ShootEmUp::Components::Score>();
        
        // UI components
        coordinator.RegisterComponent<Components::UIElement>();
        coordinator.RegisterComponent<Components::UIText>();
        coordinator.RegisterComponent<Components::UIButton>();
        coordinator.RegisterComponent<Components::UISlider>();
        coordinator.RegisterComponent<Components::UIInputField>();
        coordinator.RegisterComponent<Components::UIPanel>();
        coordinator.RegisterComponent<Components::UICheckbox>();
        coordinator.RegisterComponent<Components::UIDropdown>();
        
        logger.info("GameInitializer", "All components registered successfully");
        
    } catch (const std::exception& e) {
        logger.error("GameInitializer", std::string("Error during component registration: ") + e.what());
        throw;
    }
}

bool GameInitializer::RegisterSystems(ECS::Coordinator& coordinator,
                                     eng::engine::rendering::sfml::SFMLRenderer& renderer,
                                     std::shared_ptr<UISystem>* outUISystem,
                                     std::shared_ptr<RenderSystem>* outRenderSystem,
                                     SystemsManager* outSystemsManager) {
    auto& logger = rtype::core::Logger::getInstance();
    logger.info("GameInitializer", "Registering systems...");
    
    try {
        // Movement System
        auto movementSystem = coordinator.RegisterSystem<MovementSystem>(&coordinator);
        ECS::Signature movementSig;
        movementSig.set(coordinator.GetComponentType<Position>());
        movementSig.set(coordinator.GetComponentType<Velocity>());
        coordinator.SetSystemSignature<MovementSystem>(movementSig);
        movementSystem->Init();
        
        // Animation System
        auto animationSystem = coordinator.RegisterSystem<AnimationSystem>();
        animationSystem->SetCoordinator(&coordinator);
        ECS::Signature animSig;
        animSig.set(coordinator.GetComponentType<Animation>());
        animSig.set(coordinator.GetComponentType<Sprite>());
        coordinator.SetSystemSignature<AnimationSystem>(animSig);
        animationSystem->Init();
        
        // State Machine Animation System
        auto stateMachineAnimSystem = coordinator.RegisterSystem<StateMachineAnimationSystem>(&coordinator);
        ECS::Signature stateMachineSig;
        stateMachineSig.set(coordinator.GetComponentType<StateMachineAnimation>());
        stateMachineSig.set(coordinator.GetComponentType<Sprite>());
        coordinator.SetSystemSignature<StateMachineAnimationSystem>(stateMachineSig);
        stateMachineAnimSystem->Init();
        
        // Lifetime System
        auto lifetimeSystem = coordinator.RegisterSystem<LifetimeSystem>(&coordinator);
        ECS::Signature lifetimeSig;
        lifetimeSig.set(coordinator.GetComponentType<Lifetime>());
        coordinator.SetSystemSignature<LifetimeSystem>(lifetimeSig);
        lifetimeSystem->Init();
        
        // Movement Pattern System
        auto movementPatternSystem = coordinator.RegisterSystem<MovementPatternSystem>();
        movementPatternSystem->SetCoordinator(&coordinator);
        ECS::Signature patternSig;
        patternSig.set(coordinator.GetComponentType<ShootEmUp::Components::MovementPattern>());
        patternSig.set(coordinator.GetComponentType<Position>());
        coordinator.SetSystemSignature<MovementPatternSystem>(patternSig);
        movementPatternSystem->Init();
        
        // Scrolling Background System
        auto scrollingBgSystem = coordinator.RegisterSystem<ScrollingBackgroundSystem>();
        scrollingBgSystem->SetCoordinator(&coordinator);
        ECS::Signature scrollingSig;
        scrollingSig.set(coordinator.GetComponentType<ScrollingBackground>());
        scrollingSig.set(coordinator.GetComponentType<Position>());
        coordinator.SetSystemSignature<ScrollingBackgroundSystem>(scrollingSig);
        scrollingBgSystem->Init();
        
        // Boundary System
        auto boundarySystem = coordinator.RegisterSystem<BoundarySystem>();
        boundarySystem->SetCoordinator(&coordinator);
        // Window size will be set later when config is loaded
        ECS::Signature boundarySig;
        boundarySig.set(coordinator.GetComponentType<Position>());
        boundarySig.set(coordinator.GetComponentType<Boundary>());
        coordinator.SetSystemSignature<BoundarySystem>(boundarySig);
        boundarySystem->Init();
        
        // Collision System
        auto collisionSystem = coordinator.RegisterSystem<CollisionSystem>(&coordinator);
        ECS::Signature collisionSig;
        collisionSig.set(coordinator.GetComponentType<Position>());
        collisionSig.set(coordinator.GetComponentType<Collider>());
        coordinator.SetSystemSignature<CollisionSystem>(collisionSig);
        collisionSystem->Init();
        
        // Health System
        auto healthSystem = coordinator.RegisterSystem<HealthSystem>();
        healthSystem->SetCoordinator(&coordinator);
        ECS::Signature healthSig;
        healthSig.set(coordinator.GetComponentType<Health>());
        coordinator.SetSystemSignature<HealthSystem>(healthSig);
        healthSystem->Init();
        
        // Render System
        auto renderSystem = coordinator.RegisterSystem<RenderSystem>();
        renderSystem->SetCoordinator(&coordinator);
        renderSystem->SetRenderer(&renderer);
        ECS::Signature renderSig;
        renderSig.set(coordinator.GetComponentType<Position>());
        renderSig.set(coordinator.GetComponentType<Sprite>());
        coordinator.SetSystemSignature<RenderSystem>(renderSig);
        renderSystem->Init();
        
        // UI System
        auto uiSystem = coordinator.RegisterSystem<UISystem>();
        uiSystem->SetCoordinator(&coordinator);
        ECS::Signature uiSig;
        uiSig.set(coordinator.GetComponentType<Components::UIElement>());
        coordinator.SetSystemSignature<UISystem>(uiSig);
        uiSystem->Init();
        
        // Store UISystem pointer if requested
        if (outUISystem) {
            *outUISystem = uiSystem;
        }
        
        // Store RenderSystem pointer if requested
        if (outRenderSystem) {
            *outRenderSystem = renderSystem;
        }
        
        // Store all systems in SystemsManager if requested
        if (outSystemsManager) {
            outSystemsManager->movementSystem = movementSystem;
            outSystemsManager->animationSystem = animationSystem;
            outSystemsManager->stateMachineAnimSystem = stateMachineAnimSystem;
            outSystemsManager->lifetimeSystem = lifetimeSystem;
            outSystemsManager->movementPatternSystem = movementPatternSystem;
            outSystemsManager->scrollingBgSystem = scrollingBgSystem;
            outSystemsManager->boundarySystem = boundarySystem;
            outSystemsManager->collisionSystem = collisionSystem;
            outSystemsManager->healthSystem = healthSystem;
            outSystemsManager->renderSystem = renderSystem;
            outSystemsManager->uiSystem = uiSystem;
        }
        
        logger.info("GameInitializer", "All systems registered and initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        logger.error("GameInitializer", std::string("Error registering systems: ") + e.what());
        return false;
    }
}

bool GameInitializer::SetupLuaBindings(::Scripting::LuaState& luaState, 
                                      ECS::Coordinator& coordinator) {
    auto& logger = rtype::core::Logger::getInstance();
    logger.info("GameInitializer", "Setting up Lua bindings...");
    
    try {
        // Initialize Lua state
        luaState.Init();
        luaState.EnableHotReload(true);
        
        // Register component bindings
        ::Scripting::ComponentBindings::RegisterAll(luaState.GetState());
        ::Scripting::ComponentBindings::RegisterCoordinator(luaState.GetState(), &coordinator);
        
        // Register network bindings (allows runtime connection from UI)
        RType::Network::NetworkBindings::RegisterAll(luaState.GetState());
        
        logger.info("GameInitializer", "Lua bindings configured successfully");
        return true;
        
    } catch (const std::exception& e) {
        logger.error("GameInitializer", std::string("ERROR setting up Lua bindings: ") + e.what());
        return false;
    }
}

} // namespace RType::Core
