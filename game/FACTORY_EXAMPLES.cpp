// ===================================================================
// EXEMPLE D'UTILISATION DES FACTORIES DANS Game.cpp
// ===================================================================
// Remplacez les anciennes fonctions CreateEnemy/CreateMissile par les factories

// ========== ANCIEN CODE (à remplacer) ==========
/*
    enemySpawnTimer += deltaTime;
    if (enemySpawnTimer >= enemySpawnInterval) {
        enemySpawnTimer = 0.0f;
        float spawnY = 100.0f + (rand() % 800);
        MovementPattern::Type patterns[] = {
            MovementPattern::Type::STRAIGHT,
            MovementPattern::Type::SINE_WAVE,
            MovementPattern::Type::ZIGZAG,
            MovementPattern::Type::CIRCULAR,
            MovementPattern::Type::DIAGONAL_DOWN,
            MovementPattern::Type::DIAGONAL_UP
        };
        CreateEnemy(1920.0f + 50.0f, spawnY, patterns[rand() % 6]);
    }
*/

// ========== NOUVEAU CODE (avec factories) ==========

// 1. Dans la boucle de jeu, remplacer le spawn d'ennemis :

enemySpawnTimer += deltaTime;
if (enemySpawnTimer >= enemySpawnInterval) {
    enemySpawnTimer = 0.0f;
    float spawnY = 100.0f + (rand() % 800);
    
    // Choisir un type d'ennemi aléatoire
    EnemyTag::Type enemyTypes[] = {
        EnemyTag::Type::BASIC,
        EnemyTag::Type::ZIGZAG,
        EnemyTag::Type::SINE_WAVE,
        EnemyTag::Type::KAMIKAZE,
        EnemyTag::Type::TURRET
    };
    
    int randomIndex = rand() % 5;
    ECS::Entity enemy = EnemyFactory::CreateEnemy(
        gCoordinator,
        enemyTypes[randomIndex],
        1920.0f + 50.0f,
        spawnY,
        enemyTexture.get(),
        allSprites
    );
    
    RegisterEntity(enemy);
}

// 2. Pour le tir du joueur, remplacer CreateMissile :

/*
// ANCIEN CODE
if (weapon.isCharging && !input->IsKeyDown(rtype::engine::KeyCode::Space)) {
    if (weapon.chargeTime >= weapon.minChargeTime) {
        int chargeLevel = std::min(5, static_cast<int>((weapon.chargeTime / weapon.maxChargeTime) * 5) + 1);
        CreateMissile(playerPos.x + 99.0f, playerPos.y + 25.0f, true, chargeLevel);
    } else {
        CreateMissile(playerPos.x + 99.0f, playerPos.y + 30.0f, false, 0);
    }
}
*/

// NOUVEAU CODE
if (weapon.isCharging && !input->IsKeyDown(rtype::engine::KeyCode::Space)) {
    ECS::Entity projectile;
    
    if (weapon.chargeTime >= weapon.minChargeTime) {
        // Tir chargé
        int chargeLevel = std::min(5, static_cast<int>((weapon.chargeTime / weapon.maxChargeTime) * 5) + 1);
        projectile = ProjectileFactory::CreateChargedProjectile(
            gCoordinator,
            playerPos.x + 99.0f,
            playerPos.y + 25.0f,
            chargeLevel,
            missileTexture.get(),
            allSprites,
            true,
            player
        );
    } else {
        // Tir normal
        projectile = ProjectileFactory::CreateNormalProjectile(
            gCoordinator,
            playerPos.x + 99.0f,
            playerPos.y + 30.0f,
            missileTexture.get(),
            allSprites,
            true,
            player
        );
    }
    
    RegisterEntity(projectile);
    weapon.isCharging = false;
    weapon.chargeTime = 0.0f;
    weapon.lastFireTime = totalTime;
}

// 3. Pour des armes avancées (SPREAD_SHOT, etc.) :

void FireSpreadShot(float x, float y) {
    // Tir en éventail de 5 projectiles
    float angles[] = {-20.0f, -10.0f, 0.0f, 10.0f, 20.0f};
    
    for (float angle : angles) {
        ECS::Entity proj = ProjectileFactory::CreateNormalProjectile(
            gCoordinator,
            x, y,
            missileTexture.get(),
            allSprites,
            true,
            player
        );
        
        // Ajuster la vélocité selon l'angle
        auto& vel = gCoordinator.GetComponent<Velocity>(proj);
        float radians = angle * 3.14159f / 180.0f;
        vel.vx = std::cos(radians) * 1000.0f;
        vel.vy = std::sin(radians) * 1000.0f;
        
        RegisterEntity(proj);
    }
}

// 4. Fonction d'amélioration d'arme :

void UpgradePlayerWeapon(ECS::Entity player) {
    auto& weapon = gCoordinator.GetComponent<Weapon>(player);
    
    // Cycle à travers les types d'armes
    switch (weapon.weaponType) {
        case Weapon::Type::SINGLE_SHOT:
            weapon.weaponType = Weapon::Type::DOUBLE_SHOT;
            weapon.projectileCount = 2;
            weapon.spreadAngle = 5.0f;
            break;
        case Weapon::Type::DOUBLE_SHOT:
            weapon.weaponType = Weapon::Type::SPREAD_SHOT;
            weapon.projectileCount = 5;
            weapon.spreadAngle = 15.0f;
            break;
        case Weapon::Type::SPREAD_SHOT:
            weapon.weaponType = Weapon::Type::LASER;
            weapon.fireRate = 0.1f;
            break;
        default:
            weapon.weaponType = Weapon::Type::SINGLE_SHOT;
            weapon.projectileCount = 1;
            break;
    }
    
    weapon.level++;
}

// 5. Spawner de vagues d'ennemis :

void SpawnEnemyWave(int waveNumber) {
    // Vague 1: Seulement des BASIC
    if (waveNumber == 1) {
        for (int i = 0; i < 5; i++) {
            ECS::Entity enemy = EnemyFactory::CreateBasicEnemy(
                gCoordinator,
                1920.0f + i * 100.0f,
                200.0f + i * 80.0f,
                enemyTexture.get(),
                allSprites
            );
            RegisterEntity(enemy);
        }
    }
    // Vague 2: Mix BASIC et ZIGZAG
    else if (waveNumber == 2) {
        for (int i = 0; i < 3; i++) {
            ECS::Entity basic = EnemyFactory::CreateBasicEnemy(
                gCoordinator, 1920.0f + i * 100.0f, 200.0f, enemyTexture.get(), allSprites
            );
            ECS::Entity zigzag = EnemyFactory::CreateZigZagEnemy(
                gCoordinator, 1920.0f + i * 100.0f, 500.0f, enemyTexture.get(), allSprites
            );
            RegisterEntity(basic);
            RegisterEntity(zigzag);
        }
    }
    // Vague 3: Boss!
    else if (waveNumber == 3) {
        ECS::Entity boss = EnemyFactory::CreateBossEnemy(
            gCoordinator, 1500.0f, 540.0f, enemyTexture.get(), allSprites
        );
        RegisterEntity(boss);
    }
}

// 6. Test de tous les types de projectiles (pour debug) :

void TestAllProjectileTypes() {
    float startY = 100.0f;
    ProjectileTag::Type types[] = {
        ProjectileTag::Type::NORMAL,
        ProjectileTag::Type::CHARGED,
        ProjectileTag::Type::EXPLOSIVE,
        ProjectileTag::Type::PIERCING,
        ProjectileTag::Type::LASER
    };
    
    for (auto type : types) {
        ECS::Entity proj = ProjectileFactory::CreateProjectile(
            gCoordinator,
            type,
            100.0f,
            startY,
            missileTexture.get(),
            allSprites,
            true,
            0,
            3  // level
        );
        RegisterEntity(proj);
        startY += 50.0f;
    }
}
