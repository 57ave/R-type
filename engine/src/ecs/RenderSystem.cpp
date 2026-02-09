#include <ecs/RenderSystem.hpp>
#include <iostream>

namespace ECS {

    RenderSystem::RenderSystem() {
        // Constructor
    }

    void RenderSystem::Init() {
        std::cout << "[RenderSystem] Initialized" << std::endl;
    }

    void RenderSystem::Update(float deltaTime) {
        if (!m_Renderer || !m_Coordinator) {
            return;
        }
        
        // Get entities sorted by layer (back to front)
        auto sortedEntities = GetSortedEntitiesByLayer();
        
        // Render each entity
        for (auto entity : sortedEntities) {
            auto& transform = m_Coordinator->GetComponent<eng::engine::ECS::Transform>(entity);
            auto& sprite = m_Coordinator->GetComponent<eng::engine::ECS::Sprite>(entity);
            
            // Skip invisible sprites
            if (!sprite.visible) {
                continue;
            }
            
            // Ensure sprite is loaded
            EnsureSpriteLoaded(sprite);
            
            // Skip if sprite failed to load
            if (!sprite.sprite) {
                continue;
            }
            
            // Convert ECS transform to rendering transform
            auto renderTransform = ToRenderingTransform(transform);
            
            // Draw the sprite
            m_Renderer->draw(*sprite.sprite, renderTransform);
        }
    }

    void RenderSystem::Shutdown() {
        std::cout << "[RenderSystem] Shutdown" << std::endl;
    }

    void RenderSystem::SetRenderer(eng::engine::rendering::IRenderer* renderer) {
        m_Renderer = renderer;
    }

    void RenderSystem::SetResourceManager(eng::core::ResourceManager* resourceManager) {
        m_ResourceManager = resourceManager;
    }

    void RenderSystem::SetCoordinator(Coordinator* coordinator) {
        m_Coordinator = coordinator;
    }

    eng::engine::rendering::Transform RenderSystem::ToRenderingTransform(const eng::engine::ECS::Transform& ecsTransform) {
        eng::engine::rendering::Transform renderTransform;
        renderTransform.position = {ecsTransform.x, ecsTransform.y};
        renderTransform.rotation = ecsTransform.rotation;
        renderTransform.scale = {1.0f, 1.0f};  // Default scale
        return renderTransform;
    }

    void RenderSystem::EnsureSpriteLoaded(eng::engine::ECS::Sprite& sprite) {
        // If sprite is already loaded, nothing to do
        if (sprite.sprite) {
            return;
        }
        
        // If no resource manager, can't load
        if (!m_ResourceManager) {
            std::cerr << "[RenderSystem] Error: No ResourceManager set, cannot load sprite: " 
                      << sprite.texturePath << std::endl;
            return;
        }
        
        // If no texture path, nothing to load
        if (sprite.texturePath.empty()) {
            return;
        }
        
        // Try to load sprite from resource manager
        try {
            sprite.sprite = m_ResourceManager->getSprite(sprite.texturePath);
            if (!sprite.sprite) {
                std::cerr << "[RenderSystem] Warning: Failed to load sprite: " 
                          << sprite.texturePath << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "[RenderSystem] Error loading sprite '" << sprite.texturePath 
                      << "': " << e.what() << std::endl;
        }
    }

    std::vector<Entity> RenderSystem::GetSortedEntitiesByLayer() {
        // Copy entities to a vector for sorting
        std::vector<Entity> entities(mEntities.begin(), mEntities.end());
        
        // Sort by layer (lower layer = drawn first = background)
        std::sort(entities.begin(), entities.end(), [this](Entity a, Entity b) {
            const auto& spriteA = m_Coordinator->GetComponent<eng::engine::ECS::Sprite>(a);
            const auto& spriteB = m_Coordinator->GetComponent<eng::engine::ECS::Sprite>(b);
            return spriteA.layer < spriteB.layer;
        });
        
        return entities;
    }

} // namespace ECS
