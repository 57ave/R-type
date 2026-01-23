# 🎮 R-TYPE - REFONTE COMPLÈTE DU JEU

## 🎯 OBJECTIF PRINCIPAL
Refaire **complètement** le jeu R-Type en utilisant l'architecture ECS existante du dossier `engine/`, avec un système de networking basé sur `server/src/main_improved.cpp`, et une configuration **ENTIÈREMENT** pilotée par des scripts Lua avec hot-reload.

---

## 📋 RÈGLES ABSOLUES

### ❌ INTERDICTIONS STRICTES
1. **NE PAS TOUCHER** à `server/src/main_improved.cpp` - le serveur est déjà fonctionnel
2. **NE PAS CRÉER** de fichiers .md ou documentation (sauf demande explicite)
3. **AUCUNE DONNÉE EN DUR** dans le code C++ (chemins, valeurs de gameplay, positions, etc.)
4. **PAS DE MAIN MONOLITHIQUE** - architecture propre et séparée obligatoire
5. **NE PAS** créer 2 jeux différents pour solo/multi - UN SEUL JEU

### ✅ OBLIGATIONS ABSOLUES
1. **TOUT** doit utiliser l'ECS du dossier `engine/` (Systems, Components, Coordinator)
2. **MAXIMUM** de configuration en Lua (gameplay, UI, assets, ennemis, armes, niveaux, boss, patterns, waves...)
3. **Hot-reload** fonctionnel au démarrage pour tous les scripts Lua
4. Architecture de fichiers **professionnelle** (comme un vrai jeu vidéo)
5. Utiliser les assets existants dans `game/assets/` (sprites, sons, fonts)

---

## 🏗️ ARCHITECTURE TECHNIQUE

### Structure ECS Existante (À UTILISER)
```
engine/
├── include/
│   ├── ecs/              # ECS core (Coordinator, EntityManager, ComponentManager, SystemManager)
│   ├── components/       # Components disponibles (Position, Velocity, Sprite, Health, Collider, etc.)
│   ├── systems/          # Systems disponibles (Movement, Collision, Animation, Render, Audio, etc.)
│   ├── scripting/        # LuaState.hpp - système Lua avec hot-reload
│   ├── network/          # NetworkClient, NetworkServer, RoomManager, Room, Protocol
│   └── rendering/        # Window, Renderer SFML
```

### Structure du Jeu à Créer
```
game/
├── CMakeLists.txt        # Build config (à adapter)
├── main.cpp              # Point d'entrée minimaliste
├── include/
│   ├── core/
│   │   ├── Game.hpp               # Classe principale du jeu
│   │   └── GameConfig.hpp         # Config chargée depuis Lua
│   ├── states/
│   │   ├── GameState.hpp          # Interface état de jeu
│   │   ├── MainMenuState.hpp      # Menu principal
│   │   ├── MultiplayerMenuState.hpp # Menu multi (rooms)
│   │   ├── LobbyState.hpp         # Lobby de room
│   │   ├── SettingsState.hpp      # Menu paramètres
│   │   ├── PlayState.hpp          # État de jeu (gameplay)
│   │   └── PauseState.hpp         # Menu pause
│   ├── managers/
│   │   ├── StateManager.hpp       # Gestion des états
│   │   ├── NetworkManager.hpp     # Interface réseau client
│   │   ├── LevelManager.hpp       # Gestion des niveaux/stages
│   │   └── WaveManager.hpp        # Gestion des vagues d'ennemis
│   ├── factories/
│   │   ├── EntityFactory.hpp      # Création d'entités depuis Lua
│   │   ├── EnemyFactory.hpp       # Création ennemis configurés
│   │   └── WeaponFactory.hpp      # Création armes/projectiles
│   └── systems/
│       ├── PlayerInputSystem.hpp  # Input joueur (local + réseau)
│       ├── WeaponSystem.hpp       # Gestion armes/tirs
│       ├── EnemyAISystem.hpp      # IA ennemis (patterns Lua)
│       ├── WaveSpawnSystem.hpp    # Spawn des vagues
│       └── NetworkSyncSystem.hpp  # Sync réseau
├── src/                   # Implémentations .cpp
└── assets/
    ├── scripts/           # TOUS LES SCRIPTS LUA ICI
    │   ├── init.lua              # Point d'entrée Lua
    │   ├── config/
    │   │   ├── game_config.lua        # Config générale
    │   │   ├── player_config.lua      # Stats joueur
    │   │   ├── weapons_config.lua     # Toutes les armes
    │   │   ├── enemies_config.lua     # Tous les ennemis
    │   │   ├── bosses_config.lua      # Tous les boss
    │   │   └── assets_paths.lua       # Chemins sprites/sons
    │   ├── levels/
    │   │   ├── level1.lua             # Stage 1 (background, musique, vagues)
    │   │   ├── level2.lua             # Stage 2
    │   │   └── ...
    │   ├── waves/
    │   │   ├── level1_wave1.lua       # Vague 1 du niveau 1
    │   │   ├── level1_wave2.lua
    │   │   ├── level1_boss.lua        # Boss niveau 1
    │   │   └── ...
    │   ├── patterns/
    │   │   ├── enemy_patterns.lua     # Patterns mouvement ennemis
    │   │   └── bullet_patterns.lua    # Patterns tirs ennemis
    │   └── ui/
    │       ├── main_menu.lua          # Config menu principal
    │       ├── multiplayer_menu.lua   # Config menu multi
    │       └── hud.lua                # Config HUD in-game
    ├── enemies/           # Sprites ennemis (r-typesheet*.png)
    ├── players/           # Sprites joueurs (r-typesheet1.png, r-typesheet42.png)
    ├── vfx/               # Effets visuels (explosions, sons)
    ├── fonts/             # Polices texte
    ├── sounds/            # Sons du jeu
    └── config/
        └── settings.json  # Paramètres joueur sauvegardés (volume, contrôles, etc.)
```

---

## 🎮 FONCTIONNALITÉS OBLIGATOIRES

### 1. Menu System (États de Jeu)

#### Menu Principal
- **Bouton PLAY** → Lance le jeu en mode solo (utilise quand même le serveur en local ou logique serveur intégrée)
- **Bouton MULTIPLAYER** → Menu de gestion des rooms
- **Bouton SETTINGS** → Menu paramètres
- **Bouton QUIT** → Quitter

#### Menu Multiplayer
- **Liste des rooms disponibles** (communicant avec `NetworkServer` via protocole existant)
- **Bouton CREATE ROOM** → Créer une room (nom, 2-4 joueurs max)
- **Bouton JOIN ROOM** → Rejoindre une room sélectionnée
- **Chat inter-room** (si possible, optionnel mais souhaité)
- Communication via `RoomManager` du serveur

#### Lobby (Après join/create room)
- **Affichage des joueurs connectés** dans la room
- **Bouton START** (visible uniquement pour l'hôte)
- **Bouton LEAVE** → Quitter la room
- Synchronisation réseau temps réel

#### Menu Settings
- Volume musique/SFX (sliders)
- Configuration contrôles
- **Sauvegarde dans `assets/config/settings.json`** (local par joueur)

#### Menu Pause (ESC en jeu)
- Bouton Resume
- Bouton Settings
- Bouton Quit to Menu

### 2. Gameplay Core

#### Joueur
- **Vaisseau spatial** avec animations (spritesheet `r-typesheet1.png` et `r-typesheet42.png`)
- **Déplacement** : 8 directions (ZQSD ou flèches)
- **Tir normal** : Appui simple sur bouton de tir
- **Tir chargé** : Maintien bouton de tir (5 niveaux de charge, visuels différents)
- **Barre de vie** (3-5 HP selon config Lua)
- **Animations** : idle, tilt haut/bas, explosions mort

#### Ennemis
- **Multiples types** définis en Lua (`enemies_config.lua`)
- **Patterns de mouvement** scriptés en Lua :
  - Ligne droite
  - Zigzag
  - Sine wave
  - Spirale
  - Kamikaze (vers joueur)
  - Personnalisés...
- **Patterns de tir** scriptés en Lua :
  - Tir droit
  - Tir en éventail
  - Tir rotatif
  - Tir ciblé joueur
  - Personnalisés...
- **Animations** : mouvement, tir, mort/explosion
- **Sprites** : Utiliser les `r-typesheet*.png` dans `game/assets/enemies/`

#### Boss
- **1 boss par niveau** minimum
- **Patterns complexes** scriptés en Lua
- **Phases multiples** (optionnel mais apprécié)
- **Points faibles** (colliders spécifiques)
- **Animations** et sons dédiés

#### Système de Niveaux (Stages)
- **Chaque niveau** défini dans `assets/scripts/levels/levelX.lua` :
  - Background parallax infini unique
  - Musique de fond unique
  - Liste des vagues (`wave1`, `wave2`, ..., `boss`)
  - Conditions de victoire (tuer le boss)
- **Progression automatique** : Niveau 1 → Boss 1 → Niveau 2 → Boss 2 → ...
- **Scrolling horizontal** infini (fond qui défile)

#### Système de Vagues
- **Vagues définies en Lua** (`assets/scripts/waves/`)
- Format exemple :
```lua
-- level1_wave1.lua
return {
    enemies = {
        { type = "basic", spawn_time = 0.0, x = 1920, y = 200 },
        { type = "zigzag", spawn_time = 2.0, x = 1920, y = 400 },
        -- ...
    },
    duration = 30.0, -- secondes
}
```
- **WaveManager** charge et spawn selon le timing

#### Conditions de Victoire/Défaite
- **VICTOIRE** : Finir tous les niveaux (tuer tous les boss)
- **DÉFAITE** : Tous les joueurs morts
- **Écrans de fin** appropriés (Victory Screen, Game Over Screen)

#### Score & UI
- **Score** affiché en HUD
- **Best Score** sauvegardé localement
- **Barre de vie** du joueur
- **Niveau actuel** et vague
- **Charge du tir** (indicateur visuel)

### 3. Networking (Basé sur `server/src/main_improved.cpp`)

#### Serveur (NE PAS MODIFIER)
- Utilise `server/src/main_improved.cpp` tel quel
- Gère les rooms via `RoomManager`
- Protocole défini dans `engine/include/network/RTypeProtocol.hpp`
- Simulation serveur avec entités, collisions, spawn

#### Client (À CRÉER)
- **NetworkManager** qui communique avec le serveur
- **Envoi d'inputs** au serveur (pas de simulation locale)
- **Réception des snapshots** du serveur (états des entités)
- **Interpolation/prédiction** (optionnel mais recommandé)
- **Mode Solo** : 
  - Option 1 (préférée) : Lancer un serveur en local (thread séparé)
  - Option 2 : Intégrer la logique serveur directement dans le client

#### Protocole
- Utiliser `GamePacketType` existant :
  - `CLIENT_HELLO`, `SERVER_WELCOME`
  - `ROOM_LIST`, `CREATE_ROOM`, `JOIN_ROOM`, `GAME_START`
  - `CLIENT_INPUT`, `WORLD_SNAPSHOT`
  - `CHAT_MESSAGE`
  - Etc.

### 4. Lua Scripting (MAXIMUM DE CONFIG)

#### Bindings Lua à Créer
```cpp
// Exemple de bindings nécessaires
lua["create_enemy"] = [&](std::string type, float x, float y) { /* ... */ };
lua["create_projectile"] = [&](/* ... */) { /* ... */ };
lua["register_pattern"] = [&](/* ... */) { /* ... */ };
lua["load_sprite"] = [&](std::string path) { /* ... */ };
// Etc.
```

#### Ce qui DOIT être en Lua
- ✅ Chemins de tous les assets (sprites, sons, fonts)
- ✅ Stats joueur (vitesse, HP, cadence de tir, dégâts)
- ✅ Toutes les armes (normal, chargé niv 1-5, ennemis)
- ✅ Tous les ennemis (HP, vitesse, sprite, pattern, tir)
- ✅ Tous les boss (phases, patterns, HP, etc.)
- ✅ Tous les patterns de mouvement
- ✅ Tous les patterns de tir
- ✅ Tous les niveaux (background, musique, vagues)
- ✅ Toutes les vagues (spawn timing, ennemis)
- ✅ Configuration UI (positions menus, textes, tailles)
- ✅ Paramètres de jeu (scrolling speed, difficulty, etc.)

#### Hot-Reload
- Au **démarrage du jeu** : Charger tous les scripts Lua
- Utiliser `LuaState::CheckForChanges()` pour détecter modifications
- Recharger automatiquement si fichier modifié (pendant dev)

---

## 🔧 IMPLÉMENTATION TECHNIQUE

### Étape 1 : Setup de Base
1. Créer la structure de fichiers complète
2. Configurer `game/CMakeLists.txt` pour compiler avec l'engine
3. Créer `main.cpp` minimaliste qui instancie `Game`
4. Créer classe `Game` avec initialization ECS + LuaState

### Étape 2 : State Manager
1. Interface `GameState` avec `onEnter`, `onExit`, `update`, `render`
2. `StateManager` avec stack d'états
3. Implémenter `MainMenuState` basique (test)

### Étape 3 : Lua Loading
1. Créer tous les fichiers Lua de config de base
2. Bindings Lua pour créer entités, charger assets
3. Charger `init.lua` qui charge toutes les configs

### Étape 4 : Menu System
1. Implémenter tous les menus avec `UISystem` existant
2. Lecture config menus depuis Lua
3. Sauvegarde/chargement settings JSON

### Étape 5 : Networking Client
1. `NetworkManager` qui wrap `NetworkClient`
2. Connexion au serveur, gestion protocole
3. Intégration rooms (liste, create, join, lobby)

### Étape 6 : Gameplay Core
1. `PlayState` avec ECS setup
2. `PlayerInputSystem` (clavier → envoi réseau)
3. `NetworkSyncSystem` (snapshot → update entités)
4. Création joueur avec animations

### Étape 7 : Weapons & Shooting
1. `WeaponSystem` (tir normal + chargé)
2. Charger configs armes depuis Lua
3. Projectiles avec collisions

### Étape 8 : Enemies & AI
1. `EnemyFactory` qui lit Lua configs
2. `EnemyAISystem` qui exécute patterns Lua
3. Tirs ennemis avec patterns

### Étape 9 : Waves & Levels
1. `LevelManager` qui charge levels Lua
2. `WaveManager` qui spawn selon timing
3. Transition entre vagues et niveaux

### Étape 10 : Boss & Win Conditions
1. Boss scriptés en Lua (patterns complexes)
2. Détection victoire (boss mort → niveau suivant)
3. Détection défaite (tous joueurs morts)
4. Écrans de fin

### Étape 11 : Polish
1. Parallax backgrounds (système déjà dans engine)
2. Particules/explosions/VFX
3. Sons et musiques
4. Score et best score
5. HUD complet

### Étape 12 : Mode Solo
1. Lancer serveur en thread séparé pour mode solo
2. Ou intégrer logique serveur dans client

---

## 📦 ASSETS EXISTANTS À UTILISER

### Sprites
- **Joueurs** : `game/assets/players/r-typesheet1.png`, `r-typesheet42.png`
- **Ennemis** : `game/assets/enemies/r-typesheet*.png` (40+ spritesheets)
- **Background** : `game/assets/background.png` (+ possibilité d'en ajouter)

### Sons
- `game/assets/vfx/shoot.ogg` - Tir normal
- `game/assets/vfx/laser_bot.ogg` - Tir chargé
- `game/assets/vfx/Boom.ogg` - Explosion
- `game/assets/vfx/damage.ogg` - Dégâts
- `game/assets/vfx/multi_laser_bot.ogg` - Multi-tir

### Fonts
- Polices dans `game/assets/fonts/` pour UI

---

## 🎯 PRIORITÉS

### Phase 1 (CRITIQUE)
1. Architecture de base + State Manager
2. Menu principal fonctionnel
3. Menu Multiplayer + Rooms + Lobby (avec networking)
4. Gameplay de base (1 joueur, tir, ennemis simples)

### Phase 2 (IMPORTANT)
5. Système de vagues et niveaux
6. Boss fights
7. Conditions victoire/défaite
8. Mode solo fonctionnel

### Phase 3 (POLISH)
9. Menu settings + sauvegarde
10. HUD complet + score
11. Tous les patterns ennemis/tirs
12. Multiples niveaux complets

---

## ⚠️ CONTRAINTES IMPORTANTES

1. **Utiliser UNIQUEMENT les systèmes ECS existants** (dans `engine/include/systems/`)
2. Si besoin de nouveaux Components/Systems, **me prévenir AVANT** de les créer
3. **Ne pas dupliquer de code** - réutiliser au maximum
4. **Séparer logique et données** - tout configurable en Lua
5. **Pas de magic numbers** - tout en constantes/configs
6. Le code doit être **maintenable** et **extensible**
7. **Commentaires clairs** en anglais dans le code

---

## 📝 EXEMPLE DE CODE ATTENDU

### main.cpp
```cpp
#include "core/Game.hpp"

int main() {
    Game game;
    if (!game.initialize()) {
        return -1;
    }
    game.run();
    return 0;
}
```

### Game.hpp (Structure minimale)
```cpp
class Game {
public:
    bool initialize();
    void run();
    void shutdown();
    
private:
    void loadLuaConfigs();
    void setupECS();
    void handleEvents();
    void update(float dt);
    void render();
    
    std::unique_ptr<eng::ecs::Coordinator> coordinator_;
    std::unique_ptr<StateManager> stateManager_;
    std::unique_ptr<NetworkManager> networkManager_;
    // ...
};
```

### Exemple Lua Config (enemies_config.lua)
```lua
return {
    basic = {
        sprite = "assets/enemies/r-typesheet10.png",
        animation = { frames = 2, speed = 0.2 },
        hp = 1,
        speed = 200,
        points = 100,
        pattern = "straight_left",
        shoot_pattern = "single_forward",
        shoot_rate = 2.0,
    },
    zigzag = {
        sprite = "assets/enemies/r-typesheet11.png",
        -- ...
    },
    -- ...
}
```

---

## 🚀 COMMANDES DE BUILD

```bash
# Build
mkdir -p build && cd build
cmake .. && make game -j4

# Run server
./server

# Run game (client)
./game
```

---

## ✅ CHECKLIST FINALE

Avant de considérer le projet terminé, vérifier :

- [ ] Le jeu compile sans erreurs ni warnings
- [ ] Mode multiplayer fonctionnel (rooms, lobby, sync réseau)
- [ ] Mode solo fonctionnel
- [ ] Tous les menus fonctionnels et navigables
- [ ] Au moins 2 niveaux complets (vagues + boss)
- [ ] Tir normal et chargé fonctionnels
- [ ] Au moins 3 types d'ennemis avec patterns différents
- [ ] Boss scriptés en Lua
- [ ] Conditions victoire/défaite implémentées
- [ ] HUD complet (vie, score, niveau)
- [ ] Settings sauvegardés/chargés correctement
- [ ] Hot-reload Lua fonctionnel au démarrage
- [ ] Parallax background scrolling infini
- [ ] Sons et musiques intégrés
- [ ] Architecture propre et séparée
- [ ] Aucune donnée en dur dans le code C++

---

**TU AS MAINTENANT TOUS LES ÉLÉMENTS POUR CRÉER UN R-TYPE COMPLET, RÉSEAU, SCRIPTÉ EN LUA, AVEC UNE ARCHITECTURE PROPRE. GO ! 🚀🎮**
