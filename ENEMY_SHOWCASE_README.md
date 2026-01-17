# 🎮 R-Type - Système de Test des Ennemis et Boss

## ✅ Modifications Effectuées

### 1. Réorganisation des Fichiers de Configuration

**Avant :**
```
game/assets/scripts/config/
├── bosses_config.lua
├── enemies_config.lua
├── gameplay_config.lua
├── master_config.lua
├── powerups_config.lua
├── stages_config.lua
└── weapons_config.lua
```

**Après :**
```
assets/scripts/
├── bosses_config.lua        ← Déplacé
├── enemies_config.lua       ← Déplacé
├── gameplay_config.lua      ← Déplacé
├── master_config.lua        ← Déplacé
├── powerups_config.lua      ← Déplacé
├── stages_config.lua        ← Déplacé
├── weapons_config.lua       ← Déplacé
├── enemy_showcase.lua       ← NOUVEAU - Système de test
└── init.lua                 ← NOUVEAU - Initialisation principale
```

### 2. Nouveau Système de Showcase

**Fichier créé : `assets/scripts/enemy_showcase.lua`**

Ce système permet de tester visuellement tous les ennemis et boss du jeu avec leurs sprites et animations.

#### Fonctionnalités :

- ✨ Affichage automatique de tous les ennemis un par un
- 🎯 Test de sprites et animations
- 📊 Support de 15+ types d'ennemis
- 🦴 Support de 3+ boss
- 🔄 Mode automatique ou manuel

### 3. Système d'Initialisation Centralisé

**Fichier créé : `assets/scripts/init.lua`**

Ce fichier charge automatiquement toutes les configurations et initialise le système de showcase.

## 🚀 Comment Utiliser

### Mode Automatique (Par Défaut)

Quand vous lancez le jeu en mode solo, le showcase démarre **automatiquement** :

```bash
./r-type_game
```

Le système va :
1. Charger toutes les configurations
2. Activer le mode showcase
3. Afficher les ennemis un par un avec un intervalle de 1.5 secondes
4. Afficher les informations de chaque ennemi dans la console

### Mode Manuel

Pour désactiver le showcase automatique, modifiez dans `assets/scripts/init.lua` :

```lua
AUTO_START_SHOWCASE = false  -- Mettre à false pour désactiver
```

Puis utilisez les commandes Lua pendant le jeu.

## 📋 Commandes Lua Disponibles

### Commandes Principales

```lua
-- Démarrer le showcase automatique
StartShowcase()

-- Lister tous les ennemis disponibles
ListAllEnemies()

-- Spawner un ennemi spécifique
SpawnEnemy("basic")      -- Ennemi basique
SpawnEnemy("kamikaze")   -- Kamikaze
SpawnEnemy("shooter")    -- Tireur
SpawnEnemy("stage1_boss") -- Boss du stage 1
```

### Commandes de Groupe

```lua
-- Afficher tous les ennemis basic
ShowBasicEnemies()

-- Afficher tous les ennemis medium
ShowMediumEnemies()

-- Afficher tous les boss
ShowAllBosses()

-- Afficher tous les ennemis en grille
ShowAllEnemiesGrid()
```

### Commandes Avancées

```lua
-- Spawner un ennemi à une position spécifique
TestSpecificEnemy("elite_fighter", 400)  -- Y=400

-- Activer/désactiver le showcase
ToggleShowcaseMode()
```

## 🎭 Types d'Ennemis Disponibles

### Basic (Communs)
- `basic` - Patapata (vol direct)
- `zigzag` - Ziggy (mouvement en zigzag)
- `sinewave` - Weaver (mouvement sinusoïdal)
- `kamikaze` - Crasher (charge le joueur)

### Medium
- `shooter` - Gunner (tire sur le joueur)
- `spreader` - Spreader (tirs en éventail)
- `armored` - Tank (lourdement blindé)

### Elite
- `turret` - Turret (tourelle stationnaire)
- `elite_fighter` - Ace (manœuvres évasives)
- `formation_leader` - Commander (spawn des minions)

### Special
- `carrier` - Cargo (transporte des power-ups)
- `shielded` - Barrier (protégé par bouclier)

### Boss
- `stage1_boss` - Dobkeratops
- `stage2_boss` - Gomander
- `stage3_boss` - Big Core

## 🔧 Modifications du Code C++

### game/src/Game.cpp

**Ligne ~1169** : Changement du chargement de configuration
```cpp
// AVANT
if (!luaState.LoadScript(ResolveAssetPath("assets/scripts/config/game_config.lua"))) {
    std::cerr << "Warning: Could not load game_config.lua" << std::endl;
}

// APRÈS
if (!luaState.LoadScript(ResolveAssetPath("assets/scripts/init.lua"))) {
    // Fallback + initialisation du mode solo/réseau
    ...
}
```

**Ligne ~1378** : Ajout de l'appel UpdateGame
```cpp
// Nouveau code ajouté
if (!inMenu) {
    sol::state& lua = luaState.GetState();
    sol::protected_function updateGame = lua["UpdateGame"];
    if (updateGame.valid()) {
        updateGame(deltaTime);
    }
}
```

## 📝 Sortie Console Exemple

Quand vous lancez le jeu, vous verrez :

```
========================================
🚀 INITIALISATION R-TYPE
========================================
📦 Chargement des fichiers de configuration...

--- Configuration Files ---
  Loading: assets/scripts/master_config.lua
  ✓ master_config.lua loaded successfully
  Loading: assets/scripts/enemies_config.lua
  ✓ enemies_config.lua loaded successfully
  ...

--- Enemy Showcase System ---
  Loading: assets/scripts/enemy_showcase.lua
  ✓ enemy_showcase.lua loaded successfully

========================================
✓ Configuration chargée avec succès!
========================================

[GAME] Initialisation du mode SOLO
[GAME] AUTO_START_SHOWCASE activé - Lancement du showcase

========================================
[SHOWCASE] Mode de test des ennemis ACTIVÉ
[SHOWCASE] 15 ennemis et boss seront affichés
========================================

========================================
[SHOWCASE] Test 1/15
[SHOWCASE] Basic - Patapata
========================================
[SHOWCASE] Spawning Enemy: Patapata at Y=200
[SHOWCASE] ✓ Patapata spawned successfully!
[SHOWCASE]   - Texture: enemies/r-typesheet3.png
[SHOWCASE]   - Frame size: 33x32
[SHOWCASE]   - Scale: 2.5
[SHOWCASE]   - Animation frames: 8
```

## 🎨 Création de Niveaux Personnalisés

Vous pouvez maintenant créer vos propres niveaux en Lua ! Exemple :

```lua
-- assets/scripts/levels/level1.lua

Level1 = {
    name = "First Contact",
    duration = 120,  -- 2 minutes
    
    waves = {
        {
            time = 0,
            enemies = {
                { type = "basic", x = 1920, y = 200 },
                { type = "basic", x = 1920, y = 400 },
                { type = "basic", x = 1920, y = 600 }
            }
        },
        {
            time = 10,
            enemies = {
                { type = "zigzag", x = 1920, y = 300 },
                { type = "sinewave", x = 1920, y = 500 }
            }
        },
        {
            time = 60,
            enemies = {
                { type = "formation_leader", x = 1920, y = 540 }
            }
        },
        {
            time = 115,
            boss = { type = "stage1_boss" }
        }
    }
}

function LoadLevel1()
    -- Votre code de chargement de niveau ici
    print("Level 1 loaded!")
end
```

## 🐛 Dépannage

### Le showcase ne démarre pas
1. Vérifiez que `AUTO_START_SHOWCASE = true` dans `init.lua`
2. Vérifiez que vous n'êtes pas en mode réseau (`--network`)
3. Regardez la console pour les erreurs

### Les ennemis n'apparaissent pas
1. Vérifiez que les textures sont présentes dans `game/assets/enemies/`
2. Vérifiez que les factories (EnemyFactory) sont bien chargées
3. Regardez les logs pour les erreurs de sprite

### Erreurs Lua
- Les erreurs Lua apparaissent dans la console
- Vérifiez la syntaxe de vos fichiers .lua
- Utilisez `print()` pour débugger

## 📚 Prochaines Étapes

1. **Créer vos niveaux** : Utilisez les configs d'ennemis pour créer des niveaux personnalisés
2. **Ajuster les stats** : Modifiez les fichiers de config pour équilibrer le jeu
3. **Ajouter des ennemis** : Créez de nouveaux types d'ennemis dans `enemies_config.lua`
4. **Créer des boss** : Définissez de nouveaux boss dans `bosses_config.lua`

## 🎯 Avantages du Nouveau Système

- ✅ **Itération rapide** : Testez les sprites sans recompiler
- ✅ **Visualisation facile** : Voyez tous les ennemis d'un coup d'œil
- ✅ **Configuration centralisée** : Tous les configs au même endroit
- ✅ **Extensible** : Facile d'ajouter de nouveaux ennemis/boss
- ✅ **Data-driven** : Tout en Lua, pas de code C++ à modifier

Bon développement ! 🚀
