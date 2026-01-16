# Guide d'utilisation des Factories

Ce guide explique comment utiliser les nouvelles factories pour créer différents types d'ennemis et de projectiles.

## 🎯 Composants enrichis

### EnemyTag (Tag.hpp)
```cpp
EnemyTag::Type::BASIC       // Ennemi simple horizontal
EnemyTag::Type::ZIGZAG      // Mouvement zigzag
EnemyTag::Type::SINE_WAVE   // Mouvement sinusoïdal
EnemyTag::Type::KAMIKAZE    // Fonce sur le joueur
EnemyTag::Type::TURRET      // Tourelle statique
EnemyTag::Type::BOSS        // Boss puissant
```

### ProjectileTag (Tag.hpp)
```cpp
ProjectileTag::Type::NORMAL     // Projectile simple
ProjectileTag::Type::CHARGED    // Projectile chargé
ProjectileTag::Type::EXPLOSIVE  // Explose en zone
ProjectileTag::Type::PIERCING   // Traverse les ennemis
ProjectileTag::Type::HOMING     // Suit les cibles
ProjectileTag::Type::LASER      // Rayon laser
ProjectileTag::Type::WAVE       // Effet d'onde
```

### Weapon (Weapon.hpp)
```cpp
Weapon::Type::SINGLE_SHOT       // Tir simple
Weapon::Type::DOUBLE_SHOT       // Tir double
Weapon::Type::TRIPLE_SHOT       // Tir triple
Weapon::Type::SPREAD_SHOT       // Éventail
Weapon::Type::LASER             // Laser continu
Weapon::Type::HOMING_MISSILE    // Missiles chercheurs
Weapon::Type::WAVE_BEAM         // Faisceau ondulé
Weapon::Type::CHARGE_CANNON     // Canon avec charge
```

## 🏭 Utilisation des Factories

### EnemyFactory

#### Créer un ennemi spécifique :

```cpp
// Dans Game.cpp
#include <factories/EnemyFactory.hpp>

// Créer un ennemi BASIC
ECS::Entity enemy = EnemyFactory::CreateBasicEnemy(
    gCoordinator,
    1920.0f, 400.0f,  // Position x, y
    enemyTexture.get(),
    allSprites
);

// Créer un ennemi ZIGZAG
ECS::Entity zigzag = EnemyFactory::CreateZigZagEnemy(
    gCoordinator,
    1920.0f, 300.0f,
    enemyTexture.get(),
    allSprites
);

// Créer un BOSS
ECS::Entity boss = EnemyFactory::CreateBossEnemy(
    gCoordinator,
    1920.0f, 540.0f,
    enemyTexture.get(),
    allSprites
);
```

#### Créer un ennemi avec le dispatcher générique :

```cpp
// Sélection aléatoire du type
EnemyTag::Type randomType = static_cast<EnemyTag::Type>(rand() % 6);

ECS::Entity enemy = EnemyFactory::CreateEnemy(
    gCoordinator,
    randomType,
    1920.0f, 400.0f,
    enemyTexture.get(),
    allSprites
);
```

### ProjectileFactory

#### Créer un projectile normal :

```cpp
#include <factories/ProjectileFactory.hpp>

ECS::Entity bullet = ProjectileFactory::CreateNormalProjectile(
    gCoordinator,
    playerX, playerY,
    missileTexture.get(),
    allSprites,
    true,  // isPlayerProjectile
    playerId
);
```

#### Créer un projectile chargé :

```cpp
// Niveau de charge de 1 à 5
int chargeLevel = 3;

ECS::Entity charged = ProjectileFactory::CreateChargedProjectile(
    gCoordinator,
    playerX, playerY,
    chargeLevel,
    missileTexture.get(),
    allSprites,
    true,
    playerId
);
```

#### Créer un projectile avec le dispatcher :

```cpp
// Selon le type d'arme équipée
auto& weapon = gCoordinator.GetComponent<Weapon>(player);

ProjectileTag::Type projType = ProjectileTag::Type::NORMAL;
if (weapon.weaponType == Weapon::Type::LASER) {
    projType = ProjectileTag::Type::LASER;
} else if (weapon.weaponType == Weapon::Type::CHARGE_CANNON) {
    projType = ProjectileTag::Type::CHARGED;
}

ECS::Entity projectile = ProjectileFactory::CreateProjectile(
    gCoordinator,
    projType,
    playerX, playerY,
    missileTexture.get(),
    allSprites,
    true,
    playerId,
    weapon.level  // Niveau de l'arme
);
```

## 🎨 Weapon Attachments

### Créer un attachment visuel :

```cpp
#include <components/Attachment.hpp>

ECS::Entity attachment = gCoordinator.CreateEntity();

Attachment attach;
attach.parent = playerEntity;
attach.point = Attachment::AttachmentPoint::LEFT_WING;
attach.offsetX = -20.0f;
attach.offsetY = 0.0f;
gCoordinator.AddComponent(attachment, attach);

WeaponAttachment weaponAttach;
weaponAttach.visualType = WeaponAttachment::VisualType::DOUBLE_CANNON;
weaponAttach.level = 2;
gCoordinator.AddComponent(attachment, weaponAttach);

// Ajouter sprite, position, etc...
```

## 📝 Exemple complet : Spawn d'ennemis variés

```cpp
void SpawnEnemyWave() {
    // Vague variée d'ennemis
    std::vector<EnemyTag::Type> waveComposition = {
        EnemyTag::Type::BASIC,
        EnemyTag::Type::BASIC,
        EnemyTag::Type::ZIGZAG,
        EnemyTag::Type::SINE_WAVE,
        EnemyTag::Type::KAMIKAZE,
        EnemyTag::Type::TURRET
    };

    float startY = 200.0f;
    for (auto type : waveComposition) {
        ECS::Entity enemy = EnemyFactory::CreateEnemy(
            gCoordinator,
            type,
            1920.0f + 100.0f,
            startY,
            enemyTexture.get(),
            allSprites
        );

        RegisterEntity(enemy);
        startY += 80.0f;
    }
}
```

## 🔧 Mettre à jour l'arme du joueur

```cpp
void UpgradePlayerWeapon(ECS::Entity player, Weapon::Type newType) {
    auto& weapon = gCoordinator.GetComponent<Weapon>(player);

    weapon.weaponType = newType;
    weapon.level++;

    // Ajuster les propriétés selon le type
    switch (newType) {
        case Weapon::Type::DOUBLE_SHOT:
            weapon.projectileCount = 2;
            weapon.spreadAngle = 5.0f;
            break;
        case Weapon::Type::SPREAD_SHOT:
            weapon.projectileCount = 5;
            weapon.spreadAngle = 15.0f;
            break;
        case Weapon::Type::LASER:
            weapon.fireRate = 0.1f;  // Tir rapide
            break;
        // etc...
    }
}
```

## 🎯 Conseils

1. **Utilisez les enums** au lieu des strings pour les types
2. **Les factories centralisent** la logique de création
3. **Modifiez les factories** pour ajuster les stats des ennemis/projectiles
4. **Ajoutez des systèmes** pour gérer les comportements spéciaux (homing, etc.)
5. **Les attachments** permettent des visuels modulaires

## 🚀 Prochaines étapes

- Créer un système `AttachmentSystem` pour gérer le suivi parent-enfant
- Créer un système `HomingSystem` pour les missiles à tête chercheuse
- Implémenter les explosions en zone pour les projectiles explosifs
- Ajouter des variations visuelles selon le niveau d'arme
