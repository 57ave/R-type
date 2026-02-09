#pragma once

#include "System.hpp"
#include "Components.hpp"
#include "Coordinator.hpp"
#include <rendering/IRenderer.hpp>
#include <rendering/Types.hpp>
#include <core/ResourceManager.hpp>
#include <algorithm>
#include <vector>

namespace ECS {

    /**
     * @brief RenderSystem - Renders all entities with Transform + Sprite components
     * 
     * This system bridges the ECS and the rendering engine (SFML).
     * It iterates through all entities with both Transform and Sprite components,
     * converts ECS transforms to rendering transforms, and draws them using the renderer.
     * 
     * Features:
     * - Layer-based rendering (z-order)
     * - Visibility control
     * - Automatic sprite loading from ResourceManager
     */
    class RenderSystem : public System {
    public:
        RenderSystem();
        ~RenderSystem() override = default;
        
        void Init() override;
        void Update(float deltaTime) override;
        void Shutdown() override;
        
        /**
         * @brief Set the renderer to use for drawing
         */
        void SetRenderer(eng::engine::rendering::IRenderer* renderer);
        
        /**
         * @brief Set the resource manager for loading sprites
         */
        void SetResourceManager(eng::core::ResourceManager* resourceManager);
        
        /**
         * @brief Set the coordinator for accessing components
         */
        void SetCoordinator(Coordinator* coordinator);
        
    private:
        eng::engine::rendering::IRenderer* m_Renderer = nullptr;
        eng::core::ResourceManager* m_ResourceManager = nullptr;
        Coordinator* m_Coordinator = nullptr;
        
        /**
         * @brief Convert ECS Transform to rendering Transform
         */
        eng::engine::rendering::Transform ToRenderingTransform(const eng::engine::ECS::Transform& ecsTransform);
        
        /**
         * @brief Load sprite from resource manager if not already loaded
         */
        void EnsureSpriteLoaded(eng::engine::ECS::Sprite& sprite);
        
        /**
         * @brief Sort entities by layer for proper rendering order
         */
        std::vector<Entity> GetSortedEntitiesByLayer();
    };

} // namespace ECS
