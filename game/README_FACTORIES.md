# 🎮 Système de Types Enrichi - R-Type

## ✅ Ce qui a été fait

### 1. 📦 Composants enrichis (engine/include/components/)

#### **Tag.hpp** 
- ✨ `EnemyTag` avec enum `Type` (BASIC, ZIGZAG, SINE_WAVE, KAMIKAZE, TURRET, BOSS)
- ✨ `ProjectileTag` avec enum `Type` (NORMAL, CHARGED, EXPLOSIVE, PIERCING, HOMING, LASER, WAVE)
- ➕ Propriétés supplémentaires : `scoreValue`, `aiAggressiveness`, `pierceCount`, etc.

#### **Weapon.hpp**
- ✨ `Weapon` avec enum `Type` (SINGLE_SHOT, DOUBLE_SHOT, TRIPLE_SHOT, SPREAD_SHOT, LASER, HOMING_MISSILE, WAVE_BEAM, CHARGE_CANNON)
- ➕ Propriétés pour multi-shot : `projectileCount`, `spreadAngle`, `level`
- ➕ `Damage` enrichi avec `piercing`, `maxPierceCount`, `explosionRadius`

#### **Attachment.hpp** (NOUVEAU)
- 🆕 `Attachment` pour système parent-enfant
- 🆕 `WeaponAttachment` pour les armes visuelles attachées au vaisseau
- Points d'attachement : CENTER, LEFT_WING, RIGHT_WING, etc.

### 2. 🏭 Factories (game/include/factories/ et game/src/factories/)

#### **EnemyFactory**
Méthodes pour créer différents types d'ennemis :
- `CreateBasicEnemy()` - Ennemi simple horizontal
- `CreateZigZagEnemy()` - Mouvement zigzag
- `CreateSineWaveEnemy()` - Mouvement sinusoïdal
- `CreateKamikazeEnemy()` - Fonce sur le joueur (rapide)
- `CreateTurretEnemy()` - Tourelle statique
- `CreateBossEnemy()` - Boss puissant avec plus de HP
- `CreateEnemy(type, ...)` - Dispatcher générique

#### **ProjectileFactory**
Méthodes pour créer différents types de projectiles :
- `CreateNormalProjectile()` - Projectile simple
- `CreateChargedProjectile(level)` - Projectile chargé (5 niveaux)
- `CreateExplosiveProjectile()` - Explose en zone
- `CreatePiercingProjectile(maxPierce)` - Traverse les ennemis
- `CreateHomingProjectile()` - Suit les cibles (TODO: système)
- `CreateLaserProjectile()` - Rayon laser rapide
- `CreateProjectile(type, ...)` - Dispatcher générique

### 3. 📚 Documentation

- ✅ `FACTORY_USAGE_GUIDE.md` - Guide complet d'utilisation
- ✅ `FACTORY_EXAMPLES.cpp` - Exemples de code concrets
- ✅ Ce README

## 🚀 Comment utiliser

### Spawn d'un ennemi varié

```cpp
// Choisir un type aléatoire
EnemyTag::Type types[] = {EnemyTag::Type::BASIC, EnemyTag::Type::ZIGZAG, ...};
int randomIndex = rand() % 5;

ECS::Entity enemy = EnemyFactory::CreateEnemy(
    gCoordinator,
    types[randomIndex],
    x, y,
    texture,
    spriteList
);
RegisterEntity(enemy);
```

### Tir de projectile selon l'arme

```cpp
auto& weapon = gCoordinator.GetComponent<Weapon>(player);

ProjectileTag::Type projType = ProjectileTag::Type::NORMAL;
if (weapon.weaponType == Weapon::Type::LASER) {
    projType = ProjectileTag::Type::LASER;
}

ECS::Entity proj = ProjectileFactory::CreateProjectile(
    gCoordinator, projType, x, y, texture, spriteList, true, playerId
);
RegisterEntity(proj);
```

### Créer un weapon attachment

```cpp
ECS::Entity attachment = gCoordinator.CreateEntity();

Attachment attach;
attach.parent = playerEntity;
attach.point = Attachment::AttachmentPoint::LEFT_WING;
gCoordinator.AddComponent(attachment, attach);

WeaponAttachment weaponAttach;
weaponAttach.visualType = WeaponAttachment::VisualType::DOUBLE_CANNON;
weaponAttach.level = 2;
gCoordinator.AddComponent(attachment, weaponAttach);
```

## 📁 Structure des fichiers

```
engine/include/components/
├── Tag.hpp           ✅ Enrichi avec enums
├── Weapon.hpp        ✅ Enrichi avec enums
└── Attachment.hpp    🆕 Nouveau composant

game/
├── include/factories/
│   ├── EnemyFactory.hpp      🆕
│   └── ProjectileFactory.hpp 🆕
├── src/factories/
│   ├── EnemyFactory.cpp      🆕
│   └── ProjectileFactory.cpp 🆕
├── FACTORY_USAGE_GUIDE.md    📚
├── FACTORY_EXAMPLES.cpp      📚
└── README_FACTORIES.md       📚
```

## 🎯 Avantages de cette approche

✅ **Pas de classes** - Reste dans la philosophie ECS  
✅ **Flexibilité** - Combiner n'importe quels composants  
✅ **Performance** - Data-oriented design  
✅ **Maintenabilité** - Code centralisé dans les factories  
✅ **Extensibilité** - Facile d'ajouter de nouveaux types  
✅ **Type-safety** - Enums au lieu de strings  

## 🔮 Prochaines étapes recommandées

1. **Créer un `AttachmentSystem`** pour gérer le suivi parent-enfant
2. **Créer un `HomingSystem`** pour les missiles à tête chercheuse
3. **Implémenter la logique d'explosion** pour les projectiles explosifs
4. **Ajouter des variations visuelles** selon le niveau d'arme
5. **Créer un système de power-ups** qui change le `Weapon::Type`
6. **Load enemy waves from Lua scripts** pour des niveaux configurables

## 💡 Conseils

- Les factories centralisent la **configuration des stats**
- Modifiez les factories pour **balancer le jeu**
- Utilisez les **enums** au lieu des strings
- Gardez les **composants simples** (données uniquement)
- Mettez la **logique dans les systèmes**

## 🐛 Backward Compatibility

J'ai gardé le champ `std::string enemyType` dans `EnemyTag` pour la compatibilité avec le code existant. Vous pouvez le retirer progressivement une fois tout migré vers les enums.

---

**Créé le 5 janvier 2026**  
**Architecture ECS - Factories Pattern**
