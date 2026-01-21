#pragma once

#include <memory>
#include <systems/MovementSystem.hpp>
#include <systems/AnimationSystem.hpp>
#include <systems/StateMachineAnimationSystem.hpp>
#include <systems/LifetimeSystem.hpp>
#include <systems/MovementPatternSystem.hpp>
#include <systems/ScrollingBackgroundSystem.hpp>
#include <systems/BoundarySystem.hpp>
#include <systems/CollisionSystem.hpp>
#include <systems/HealthSystem.hpp>
#include <systems/RenderSystem.hpp>
#include <systems/UISystem.hpp>

namespace RType::Core {

/**
 * @brief Gestionnaire centralisé de tous les systèmes ECS
 * 
 * Cette classe stocke les références vers tous les systèmes enregistrés
 * et fournit des méthodes pour les mettre à jour dans le bon ordre.
 */
struct SystemsManager {
    // Systèmes de gameplay
    std::shared_ptr<MovementSystem> movementSystem;
    std::shared_ptr<AnimationSystem> animationSystem;
    std::shared_ptr<StateMachineAnimationSystem> stateMachineAnimSystem;
    std::shared_ptr<LifetimeSystem> lifetimeSystem;
    std::shared_ptr<MovementPatternSystem> movementPatternSystem;
    std::shared_ptr<ScrollingBackgroundSystem> scrollingBgSystem;
    std::shared_ptr<BoundarySystem> boundarySystem;
    std::shared_ptr<CollisionSystem> collisionSystem;
    std::shared_ptr<HealthSystem> healthSystem;
    
    // Systèmes de rendu
    std::shared_ptr<RenderSystem> renderSystem;
    std::shared_ptr<UISystem> uiSystem;
    
    /**
     * @brief Met à jour tous les systèmes visuels (toujours actifs)
     */
    void UpdateVisualSystems(float dt) {
        if (scrollingBgSystem) scrollingBgSystem->Update(dt);
        if (stateMachineAnimSystem) stateMachineAnimSystem->Update(dt);
        if (animationSystem) animationSystem->Update(dt);
        if (lifetimeSystem) lifetimeSystem->Update(dt);
    }
    
    /**
     * @brief Met à jour tous les systèmes de gameplay (uniquement en jeu)
     */
    void UpdateGameplaySystems(float dt) {
        if (movementPatternSystem) movementPatternSystem->Update(dt);
        if (movementSystem) movementSystem->Update(dt);
        if (boundarySystem) boundarySystem->Update(dt);
        if (collisionSystem) collisionSystem->Update(dt);
        if (healthSystem) healthSystem->Update(dt);
    }
    
    /**
     * @brief Met à jour le système de rendu
     */
    void UpdateRenderSystem(float dt) {
        if (renderSystem) renderSystem->Update(dt);
    }
    
    /**
     * @brief Met à jour le système UI
     */
    void UpdateUISystem(float dt) {
        if (uiSystem) uiSystem->Update(dt);
    }
    
    /**
     * @brief Obtient le système de collision
     */
    std::shared_ptr<CollisionSystem> GetCollisionSystem() const {
        return collisionSystem;
    }
};

} // namespace RType::Core
