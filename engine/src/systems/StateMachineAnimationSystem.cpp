#include "systems/StateMachineAnimationSystem.hpp"
#include <components/Position.hpp>
#include "core/Logger.hpp"

StateMachineAnimationSystem::StateMachineAnimationSystem(ECS::Coordinator* coordinator)
    : m_Coordinator(coordinator) {
}

void StateMachineAnimationSystem::Init() {
    LOG_INFO("STATEMACHINEANIMATIONSYSTEM", "Initialized");
}

void StateMachineAnimationSystem::Update(float dt) {
    if (!m_Coordinator) return;

    // Update state machine animations
    for (auto entity : mEntities) {
        if (!m_Coordinator->HasComponent<StateMachineAnimation>(entity) || 
            !m_Coordinator->HasComponent<Sprite>(entity))
            continue;

        auto& anim = m_Coordinator->GetComponent<StateMachineAnimation>(entity);
        auto& sprite = m_Coordinator->GetComponent<Sprite>(entity);

        // Update transition timer
        anim.transitionTime += dt;

        // Perform column transition
        if (anim.currentColumn != anim.targetColumn && 
            anim.transitionTime >= anim.transitionSpeed) {
            
            anim.transitionTime = 0.0f;
            
            if (anim.currentColumn < anim.targetColumn) {
                anim.currentColumn++;
            } else if (anim.currentColumn > anim.targetColumn) {
                anim.currentColumn--;
            }

            // Update sprite texture rect
            eng::engine::rendering::IntRect rect;
            rect.left = anim.spriteWidth * anim.currentColumn;
            rect.top = anim.spriteHeight * anim.currentRow;
            rect.width = anim.spriteWidth;
            rect.height = anim.spriteHeight;
            
            sprite.textureRect = rect;

            // Apply to native sprite if exists
            if (sprite.sprite) {
                sprite.sprite->setTextureRect(rect);
            }
        }
    }
}

void StateMachineAnimationSystem::Shutdown() {
    LOG_INFO("STATEMACHINEANIMATIONSYSTEM", "Shutdown");
}

// C API implementation
extern "C" {
    ECS::System* CreateSystem(ECS::Coordinator* coordinator) {
        return new StateMachineAnimationSystem(coordinator);
    }
    
    void DestroySystem(ECS::System* system) {
        delete system;
    }
    
    const char* GetSystemName() {
        return "StateMachineAnimationSystem";
    }
    
    uint32_t GetSystemVersion() {
        return 1;
    }
}
