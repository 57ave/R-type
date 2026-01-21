#include <algorithm>
#include <cmath>
#include <iostream>
#include <systems/AttachmentSystem.hpp>

namespace ShootEmUp {
namespace Systems {

// ============================================================================
// FORCE POD SYSTEM
// ============================================================================

void ForcePodSystem::Update(float dt) {
    if (forcePodEntity_ == 0)
        return;

    if (!coordinator_->HasComponent<Components::ForcePod>(forcePodEntity_)) {
        forcePodEntity_ = 0;
        return;
    }

    auto& force = coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_);

    switch (force.state) {
        case Components::ForcePod::State::AttachedFront:
        case Components::ForcePod::State::AttachedBack:
            UpdateAttached(dt);
            break;
        case Components::ForcePod::State::Detached:
            UpdateDetached(dt);
            break;
        case Components::ForcePod::State::Launching:
            UpdateLaunching(dt);
            break;
        case Components::ForcePod::State::Returning:
            UpdateReturning(dt);
            break;
    }
}

void ForcePodSystem::CreateForcePod(ECS::Entity owner) {
    if (forcePodEntity_ != 0) {
        std::cout << "[ForcePodSystem] Force pod already exists!" << std::endl;
        return;
    }

    ownerEntity_ = owner;
    forcePodEntity_ = coordinator_->CreateEntity();

    // Get owner position
    float startX = 0, startY = 0;
    if (coordinator_->HasComponent<Position>(owner)) {
        auto& pos = coordinator_->GetComponent<Position>(owner);
        startX = pos.x + 100.0f;
        startY = pos.y;
    }

    // Add components
    coordinator_->AddComponent(forcePodEntity_, Position{startX, startY});
    coordinator_->AddComponent(forcePodEntity_, Velocity{0, 0});

    Components::ForcePod force;
    force.owner = owner;
    force.state = Components::ForcePod::State::Detached;
    force.level = 1;
    coordinator_->AddComponent(forcePodEntity_, force);

    std::cout << "[ForcePodSystem] Force pod created!" << std::endl;
}

void ForcePodSystem::DestroyForcePod() {
    if (forcePodEntity_ == 0)
        return;

    coordinator_->DestroyEntity(forcePodEntity_);
    forcePodEntity_ = 0;

    std::cout << "[ForcePodSystem] Force pod destroyed" << std::endl;
}

void ForcePodSystem::AttachToFront() {
    if (forcePodEntity_ == 0)
        return;

    auto& force = coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_);
    force.state = Components::ForcePod::State::AttachedFront;

    std::cout << "[ForcePodSystem] Force attached to FRONT" << std::endl;
}

void ForcePodSystem::AttachToBack() {
    if (forcePodEntity_ == 0)
        return;

    auto& force = coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_);
    force.state = Components::ForcePod::State::AttachedBack;

    std::cout << "[ForcePodSystem] Force attached to BACK" << std::endl;
}

void ForcePodSystem::Detach() {
    if (forcePodEntity_ == 0)
        return;

    auto& force = coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_);
    force.state = Components::ForcePod::State::Detached;

    std::cout << "[ForcePodSystem] Force DETACHED" << std::endl;
}

void ForcePodSystem::Launch() {
    if (forcePodEntity_ == 0)
        return;

    auto& force = coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_);

    if (force.state == Components::ForcePod::State::AttachedFront ||
        force.state == Components::ForcePod::State::AttachedBack) {
        force.state = Components::ForcePod::State::Launching;
        force.currentLaunchDistance = 0;

        // Set velocity based on attachment side
        auto& vel = coordinator_->GetComponent<Velocity>(forcePodEntity_);
        if (force.state == Components::ForcePod::State::AttachedFront) {
            vel.dx = force.launchSpeed;
        } else {
            vel.dx = -force.launchSpeed;
        }
        vel.dy = 0;

        std::cout << "[ForcePodSystem] Force LAUNCHED!" << std::endl;
    }
}

void ForcePodSystem::Recall() {
    if (forcePodEntity_ == 0)
        return;

    auto& force = coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_);

    if (force.state == Components::ForcePod::State::Detached ||
        force.state == Components::ForcePod::State::Launching) {
        force.state = Components::ForcePod::State::Returning;

        std::cout << "[ForcePodSystem] Force RECALLED" << std::endl;
    }
}

void ForcePodSystem::ToggleAttachment() {
    if (forcePodEntity_ == 0)
        return;

    auto& force = coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_);

    switch (force.state) {
        case Components::ForcePod::State::Detached:
            AttachToFront();
            break;
        case Components::ForcePod::State::AttachedFront:
            AttachToBack();
            break;
        case Components::ForcePod::State::AttachedBack:
            Detach();
            break;
        default:
            break;
    }
}

void ForcePodSystem::UpgradeForce() {
    if (forcePodEntity_ == 0)
        return;

    auto& force = coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_);

    if (force.level < 3) {
        force.level++;
        std::cout << "[ForcePodSystem] Force upgraded to level " << force.level << std::endl;
    }
}

int ForcePodSystem::GetForceLevel() const {
    if (forcePodEntity_ == 0)
        return 0;

    if (coordinator_->HasComponent<Components::ForcePod>(forcePodEntity_)) {
        return coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_).level;
    }
    return 0;
}

void ForcePodSystem::Fire() {
    if (forcePodEntity_ == 0 || !projectileCb_)
        return;

    auto& force = coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_);
    auto& pos = coordinator_->GetComponent<Position>(forcePodEntity_);

    // Determine fire directions based on level and state
    std::vector<float> angles;

    if (force.state == Components::ForcePod::State::AttachedFront) {
        angles.push_back(0);  // Forward
        if (force.level >= 2) {
            angles.push_back(30);
            angles.push_back(-30);
        }
        if (force.level >= 3) {
            angles.push_back(45);
            angles.push_back(-45);
        }
    } else if (force.state == Components::ForcePod::State::AttachedBack) {
        angles.push_back(180);  // Backward
        if (force.level >= 2) {
            angles.push_back(150);
            angles.push_back(-150);
        }
    } else {
        // Detached: fires in both directions
        angles.push_back(0);
        angles.push_back(180);
        if (force.level >= 2) {
            angles.push_back(45);
            angles.push_back(-45);
            angles.push_back(135);
            angles.push_back(-135);
        }
    }

    for (float angle : angles) {
        projectileCb_(pos.x, pos.y, angle, force.weaponType);
    }
}

void ForcePodSystem::UpdateAttached(float dt) {
    if (ownerEntity_ == 0)
        return;

    auto& force = coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_);
    auto& forcePos = coordinator_->GetComponent<Position>(forcePodEntity_);
    auto& ownerPos = coordinator_->GetComponent<Position>(ownerEntity_);

    if (force.state == Components::ForcePod::State::AttachedFront) {
        forcePos.x = ownerPos.x + force.frontOffsetX;
        forcePos.y = ownerPos.y + force.frontOffsetY;
    } else {
        forcePos.x = ownerPos.x + force.backOffsetX;
        forcePos.y = ownerPos.y + force.backOffsetY;
    }
}

void ForcePodSystem::UpdateDetached(float dt) {
    // Force floats in place, maybe slight hover
    auto& pos = coordinator_->GetComponent<Position>(forcePodEntity_);
    auto& force = coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_);

    static float hoverTime = 0;
    hoverTime += dt;

    force.floatOffsetY = std::sin(hoverTime * 3.0f) * 5.0f;
}

void ForcePodSystem::UpdateLaunching(float dt) {
    auto& force = coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_);
    auto& pos = coordinator_->GetComponent<Position>(forcePodEntity_);
    auto& vel = coordinator_->GetComponent<Velocity>(forcePodEntity_);

    force.currentLaunchDistance += std::abs(vel.dx) * dt;

    if (force.currentLaunchDistance >= force.maxLaunchDistance) {
        force.state = Components::ForcePod::State::Detached;
        vel.dx = 0;
        vel.dy = 0;
    }
}

void ForcePodSystem::UpdateReturning(float dt) {
    if (ownerEntity_ == 0) {
        auto& force = coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_);
        force.state = Components::ForcePod::State::Detached;
        return;
    }

    auto& force = coordinator_->GetComponent<Components::ForcePod>(forcePodEntity_);
    auto& forcePos = coordinator_->GetComponent<Position>(forcePodEntity_);
    auto& ownerPos = coordinator_->GetComponent<Position>(ownerEntity_);

    float dx = ownerPos.x - forcePos.x;
    float dy = ownerPos.y - forcePos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist < 50.0f) {
        // Close enough, attach
        force.state = Components::ForcePod::State::AttachedFront;
        return;
    }

    // Move towards owner
    float speed = force.returnSpeed * dt;
    forcePos.x += (dx / dist) * speed;
    forcePos.y += (dy / dist) * speed;
}

// ============================================================================
// OPTION SYSTEM
// ============================================================================

void OptionSystem::Update(float dt) {
    if (ownerEntity_ == 0)
        return;

    // Record owner position for trail
    if (coordinator_->HasComponent<Position>(ownerEntity_)) {
        auto& pos = coordinator_->GetComponent<Position>(ownerEntity_);
        RecordPosition(pos.x, pos.y);
    }

    // Update based on formation
    if (currentFormation_ == "trail") {
        UpdateTrailFormation(dt);
    } else if (currentFormation_ == "spread") {
        UpdateSpreadFormation(dt);
    } else if (currentFormation_ == "rotate") {
        UpdateRotateFormation(dt);
    } else if (currentFormation_ == "fixed") {
        UpdateFixedFormation(dt);
    }
}

void OptionSystem::AddOption() {
    if (static_cast<int>(optionEntities_.size()) >= maxOptions_) {
        std::cout << "[OptionSystem] Max options reached!" << std::endl;
        return;
    }

    ECS::Entity option = coordinator_->CreateEntity();

    // Get owner position for initial placement
    float startX = 0, startY = 0;
    if (ownerEntity_ != 0 && coordinator_->HasComponent<Position>(ownerEntity_)) {
        auto& pos = coordinator_->GetComponent<Position>(ownerEntity_);
        startX = pos.x - 50.0f * (optionEntities_.size() + 1);
        startY = pos.y;
    }

    coordinator_->AddComponent(option, Position{startX, startY});

    Components::Option opt;
    opt.owner = ownerEntity_;
    opt.optionIndex = static_cast<int>(optionEntities_.size());
    coordinator_->AddComponent(option, opt);

    optionEntities_.push_back(option);

    std::cout << "[OptionSystem] Option added! Total: " << optionEntities_.size() << std::endl;
}

void OptionSystem::RemoveOption() {
    if (optionEntities_.empty())
        return;

    ECS::Entity option = optionEntities_.back();
    optionEntities_.pop_back();

    coordinator_->DestroyEntity(option);

    std::cout << "[OptionSystem] Option removed. Total: " << optionEntities_.size() << std::endl;
}

void OptionSystem::RemoveAllOptions() {
    for (auto option : optionEntities_) {
        coordinator_->DestroyEntity(option);
    }
    optionEntities_.clear();

    std::cout << "[OptionSystem] All options removed" << std::endl;
}

void OptionSystem::SetOwner(ECS::Entity owner) {
    ownerEntity_ = owner;

    // Update all options
    for (auto option : optionEntities_) {
        if (coordinator_->HasComponent<Components::Option>(option)) {
            coordinator_->GetComponent<Components::Option>(option).owner = owner;
        }
    }
}

void OptionSystem::SetFormation(const std::string& formation) {
    currentFormation_ = formation;
    std::cout << "[OptionSystem] Formation set to: " << formation << std::endl;
}

void OptionSystem::CycleFormation() {
    if (currentFormation_ == "trail") {
        SetFormation("spread");
    } else if (currentFormation_ == "spread") {
        SetFormation("rotate");
    } else if (currentFormation_ == "rotate") {
        SetFormation("fixed");
    } else {
        SetFormation("trail");
    }
}

void OptionSystem::Fire() {
    if (!projectileCb_)
        return;

    for (auto option : optionEntities_) {
        if (coordinator_->HasComponent<Position>(option)) {
            auto& pos = coordinator_->GetComponent<Position>(option);
            projectileCb_(pos.x + 30.0f, pos.y, 0, "option_shot");
        }
    }
}

void OptionSystem::RecordPosition(float x, float y) {
    positionHistory_.push_front({x, y});

    while (static_cast<int>(positionHistory_.size()) > historyMaxSize_) {
        positionHistory_.pop_back();
    }
}

std::pair<float, float> OptionSystem::GetHistoryPosition(int framesBack) {
    if (framesBack >= static_cast<int>(positionHistory_.size())) {
        if (positionHistory_.empty())
            return {0, 0};
        return positionHistory_.back();
    }
    return positionHistory_[framesBack];
}

void OptionSystem::UpdateTrailFormation(float dt) {
    int frameDelay = 12;  // Frames between each option

    for (size_t i = 0; i < optionEntities_.size(); ++i) {
        ECS::Entity option = optionEntities_[i];

        if (!coordinator_->HasComponent<Position>(option))
            continue;

        auto& optPos = coordinator_->GetComponent<Position>(option);
        auto histPos = GetHistoryPosition(static_cast<int>((i + 1) * frameDelay));

        optPos.x = histPos.first;
        optPos.y = histPos.second;
    }
}

void OptionSystem::UpdateSpreadFormation(float dt) {
    if (ownerEntity_ == 0)
        return;
    if (!coordinator_->HasComponent<Position>(ownerEntity_))
        return;

    auto& ownerPos = coordinator_->GetComponent<Position>(ownerEntity_);

    // Predefined spread positions
    std::vector<std::pair<float, float>> offsets = {{-50, -60}, {-50, 60}, {-100, -40}, {-100, 40}};

    for (size_t i = 0; i < optionEntities_.size() && i < offsets.size(); ++i) {
        ECS::Entity option = optionEntities_[i];

        if (!coordinator_->HasComponent<Position>(option))
            continue;

        auto& optPos = coordinator_->GetComponent<Position>(option);
        optPos.x = ownerPos.x + offsets[i].first;
        optPos.y = ownerPos.y + offsets[i].second;
    }
}

void OptionSystem::UpdateRotateFormation(float dt) {
    if (ownerEntity_ == 0)
        return;
    if (!coordinator_->HasComponent<Position>(ownerEntity_))
        return;

    static float rotationAngle = 0;
    rotationAngle += 180.0f * dt;  // 180 degrees per second

    auto& ownerPos = coordinator_->GetComponent<Position>(ownerEntity_);
    float radius = 80.0f;
    int count = static_cast<int>(optionEntities_.size());

    for (size_t i = 0; i < optionEntities_.size(); ++i) {
        ECS::Entity option = optionEntities_[i];

        if (!coordinator_->HasComponent<Position>(option))
            continue;

        float angle = rotationAngle + (360.0f / count) * i;
        float rad = angle * 3.14159f / 180.0f;

        auto& optPos = coordinator_->GetComponent<Position>(option);
        optPos.x = ownerPos.x + std::cos(rad) * radius;
        optPos.y = ownerPos.y + std::sin(rad) * radius;
    }
}

void OptionSystem::UpdateFixedFormation(float dt) {
    if (ownerEntity_ == 0)
        return;
    if (!coordinator_->HasComponent<Position>(ownerEntity_))
        return;

    auto& ownerPos = coordinator_->GetComponent<Position>(ownerEntity_);

    // Fixed positions around player
    std::vector<std::pair<float, float>> offsets = {{0, -60}, {0, 60}, {-60, -30}, {-60, 30}};

    for (size_t i = 0; i < optionEntities_.size() && i < offsets.size(); ++i) {
        ECS::Entity option = optionEntities_[i];

        if (!coordinator_->HasComponent<Position>(option))
            continue;

        auto& optPos = coordinator_->GetComponent<Position>(option);
        optPos.x = ownerPos.x + offsets[i].first;
        optPos.y = ownerPos.y + offsets[i].second;
    }
}

// ============================================================================
// SHIELD SYSTEM
// ============================================================================

void ShieldSystem::Update(float dt) {
    for (auto it = activeShields_.begin(); it != activeShields_.end();) {
        ECS::Entity shieldOwner = *it;

        if (!coordinator_->HasComponent<Components::Shield>(shieldOwner)) {
            it = activeShields_.erase(it);
            continue;
        }

        auto& shield = coordinator_->GetComponent<Components::Shield>(shieldOwner);

        // Update duration
        if (shield.duration > 0) {
            shield.currentTime += dt;
            if (shield.currentTime >= shield.duration) {
                DeactivateShield(shieldOwner);
                it = activeShields_.erase(it);
                continue;
            }
        }

        // Update pulse effect
        shield.currentPulse += dt * shield.pulseSpeed;

        // Update flash timer
        if (shield.flashTimer > 0) {
            shield.flashTimer -= dt;
        }

        ++it;
    }
}

void ShieldSystem::ActivateShield(ECS::Entity owner, float duration, int hitPoints) {
    if (HasShield(owner))
        return;

    Components::Shield shield;
    shield.owner = owner;
    shield.duration = duration;
    shield.hitPoints = hitPoints;
    shield.currentTime = 0;

    coordinator_->AddComponent(owner, shield);
    activeShields_.push_back(owner);

    std::cout << "[ShieldSystem] Shield activated! Duration: " << duration << "s, HP: " << hitPoints
              << std::endl;
}

void ShieldSystem::DeactivateShield(ECS::Entity owner) {
    auto it = std::find(activeShields_.begin(), activeShields_.end(), owner);
    if (it != activeShields_.end()) {
        activeShields_.erase(it);
    }

    if (coordinator_->HasComponent<Components::Shield>(owner)) {
        coordinator_->RemoveComponent<Components::Shield>(owner);
    }

    std::cout << "[ShieldSystem] Shield deactivated" << std::endl;
}

bool ShieldSystem::HasShield(ECS::Entity owner) const {
    return std::find(activeShields_.begin(), activeShields_.end(), owner) != activeShields_.end();
}

int ShieldSystem::GetShieldHits(ECS::Entity owner) const {
    if (!coordinator_->HasComponent<Components::Shield>(owner))
        return 0;
    return coordinator_->GetComponent<Components::Shield>(owner).hitPoints;
}

void ShieldSystem::OnShieldHit(ECS::Entity owner, int damage) {
    if (!coordinator_->HasComponent<Components::Shield>(owner))
        return;

    auto& shield = coordinator_->GetComponent<Components::Shield>(owner);
    shield.hitPoints -= 1;  // Each hit reduces by 1
    shield.flashTimer = 0.2f;

    std::cout << "[ShieldSystem] Shield hit! Remaining: " << shield.hitPoints << std::endl;

    if (shield.hitPoints <= 0) {
        if (shieldBreakCb_) {
            shieldBreakCb_(owner);
        }
        DeactivateShield(owner);
    }
}

}  // namespace Systems
}  // namespace ShootEmUp
