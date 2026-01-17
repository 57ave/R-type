# 📚 R-Type - Architecture Complète de l'Engine et du Jeu

Ce document décrit l'architecture complète de l'engine R-Type, l'organisation de ses fonctions, et comment le jeu les utilise.

---

## 📋 Table des Matières

1. [Vue d'ensemble de l'architecture](#vue-densemble-de-larchitecture)
2. [Structure de l'Engine](#structure-de-lengine)
3. [Modules de l'Engine](#modules-de-lengine)
4. [Utilisation par le Jeu](#utilisation-par-le-jeu)
5. [Flux de Données](#flux-de-données)
6. [Protocole Réseau](#protocole-réseau)

---

## 🏗️ Vue d'ensemble de l'architecture

```
┌─────────────────────────────────────────────────────────────┐
│                         GAME LAYER                          │
│  ┌───────────┐  ┌───────────┐  ┌──────────────────────┐   │
│  │  Game.cpp │  │ Factories │  │  Game-specific       │   │
│  │  main.cpp │  │           │  │  Systems & Scripts   │   │
│  └───────────┘  └───────────┘  └──────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                      SHOOTEMUP MODULE                       │
│  ┌──────────────┐  ┌──────────────┐  ┌─────────────────┐  │
│  │  Components  │  │   Systems    │  │   Factories     │  │
│  │  - Weapon    │  │  - Weapon    │  │  - Enemy        │  │
│  │  - Pattern   │  │  - Pattern   │  │  - Projectile   │  │
│  │  - PowerUp   │  │  - Spawn     │  │                 │  │
│  └──────────────┘  └──────────────┘  └─────────────────┘  │
└─────────────────────────────────────────────────────────────┘
                            ▼
┌─────────────────────────────────────────────────────────────┐
│                       ENGINE CORE                           │
│  ┌─────────┐  ┌──────────┐  ┌──────────┐  ┌────────────┐  │
│  │   ECS   │  │ Rendering│  │ Network  │  │  Scripting │  │
│  │ Manager │  │  System  │  │  Client  │  │   (Lua)    │  │
│  └─────────┘  └──────────┘  └──────────┘  └────────────┘  │
└─────────────────────────────────────────────────────────────┘
```

---

## 🔧 Structure de l'Engine

### 📁 Organisation des Fichiers

```
engine/
├── include/
│   ├── ecs/                    # Entity Component System
│   ├── core/                   # Fonctionnalités de base
│   ├── rendering/              # Système de rendu (abstrait)
│   ├── network/                # Communication réseau
│   ├── scripting/              # Intégration Lua
│   ├── systems/                # Systèmes génériques
│   ├── components/             # Composants génériques
│   └── modules/shootemup/      # Module Shoot'em Up
└── src/
    └── [implémentations correspondantes]
```

---

## 🧩 Modules de l'Engine

### 1️⃣ ECS (Entity Component System)

**Localisation:** `engine/include/ecs/`

#### Fonctions Principales:

| Fichier | Fonction | Description |
|---------|----------|-------------|
| `Coordinator.hpp` | `Init()` | Initialise le coordinateur ECS |
| | `CreateEntity()` | Crée une nouvelle entité |
| | `DestroyEntity(entity)` | Détruit une entité |
| | `RegisterComponent<T>()` | Enregistre un type de composant |
| | `AddComponent<T>(entity, component)` | Ajoute un composant à une entité |
| | `GetComponent<T>(entity)` | Récupère un composant |
| | `HasComponent<T>(entity)` | Vérifie la présence d'un composant |
| | `RegisterSystem<T>()` | Enregistre un système |
| | `SetSystemSignature<T>(signature)` | Définit la signature d'un système |
| `EntityManager.hpp` | `CreateEntity()` | Gestion des IDs d'entités |
| | `DestroyEntity(entity)` | Libération des IDs |
| `ComponentManager.hpp` | `RegisterComponent<T>()` | Gestion des types de composants |
| | `AddComponent<T>()` | Stockage des composants |
| `SystemManager.hpp` | `RegisterSystem<T>()` | Gestion des systèmes |
| | `EntityDestroyed(entity)` | Notification de destruction |

**Utilisation dans le jeu:**
```cpp
// Dans Game.cpp (lignes 458-478)
gCoordinator.Init();
gCoordinator.RegisterComponent<Position>();
gCoordinator.RegisterComponent<Velocity>();
gCoordinator.RegisterComponent<Sprite>();
// ... autres composants

// Création d'entités (ligne 35+)
ECS::Entity player = gCoordinator.CreateEntity();
gCoordinator.AddComponent(player, Position{x, y});
gCoordinator.AddComponent(player, Velocity{0.0f, 0.0f});
```

---

### 2️⃣ Core - Fonctionnalités de Base

**Localisation:** `engine/include/core/`

#### Modules Core:

| Fichier | Fonction | Utilisation Jeu |
|---------|----------|-----------------|
| `Config.hpp` | Gestion configuration | Chargement paramètres jeu |
| `Logger.hpp` | `Log()`, `Error()`, `Warning()` | Debug et traçage |
| `EventBus.hpp` | `Subscribe()`, `Publish()` | Communication entre systèmes |
| `InputManager.hpp` | Gestion des entrées | Contrôles joueur |
| `ResourceManager.hpp` | Chargement ressources | Textures, sons, scripts |
| `Time.hpp` | `GetDeltaTime()`, `GetElapsedTime()` | Synchronisation du jeu |
| `SystemLoader.hpp` | Chargement dynamique de systèmes | Extension du moteur |

**Exemple d'utilisation:**
```cpp
// EventBus pour les collisions
EventBus::Publish(CollisionEvent{entityA, entityB});

// ResourceManager pour les textures
playerTexture = resourceManager.Load<Texture>("player.png");

// Time pour le deltaTime
float dt = Time::GetDeltaTime();
```

---

### 3️⃣ Rendering - Système de Rendu

**Localisation:** `engine/include/rendering/`

#### Architecture Abstraite:

```
IRenderer (interface)
    ├── SFMLRenderer (implémentation SFML)
    └── [Future: VulkanRenderer, OpenGLRenderer...]

ITexture (interface)
    └── SFMLTexture

ISprite (interface)
    └── SFMLSprite

Camera
    └── Gestion caméra 2D/3D
```

| Classe | Fonctions Principales | Usage Jeu |
|--------|----------------------|-----------|
| `SFMLWindow` | `Create()`, `PollEvents()`, `Display()` | Fenêtre principale |
| `SFMLRenderer` | `Clear()`, `Draw()`, `Present()` | Rendu des sprites |
| `SFMLSprite` | `setTexture()`, `setPosition()`, `setTextureRect()` | Affichage entités |
| `SFMLTexture` | `LoadFromFile()` | Chargement images |
| `Camera` | `SetPosition()`, `GetView()` | Vue de jeu |

**Utilisation dans Game.cpp:**
```cpp
// Ligne 50+ - Création de sprites
auto* sprite = new SFMLSprite();
sprite->setTexture(playerTexture.get());
sprite->setPosition(Vector2f(x, y));
sprite->setTextureRect(IntRect(33*2, line*17, 33, 17));
```

---

### 4️⃣ Network - Communication Réseau

**Localisation:** `engine/include/network/`

#### Composants Réseau:

| Fichier | Fonction | Description |
|---------|----------|-------------|
| `NetworkClient.hpp` | `Connect(ip, port)` | Connexion au serveur |
| | `Send(packet)` | Envoi de paquets UDP |
| | `Receive()` | Réception asynchrone |
| `NetworkServer.hpp` | `Start(port)` | Démarrage serveur |
| | `Broadcast(packet)` | Envoi à tous les clients |
| `UdpClient.hpp` | Communication UDP bas niveau | Socket ASIO |
| `UdpServer.hpp` | Serveur UDP bas niveau | Socket ASIO |
| `Protocol.hpp` | `PacketHeader`, types de packets | Structure protocole |
| `Packet.hpp` | Sérialisation/Désérialisation | Conversion données |

**Protocole R-Type (voir Protocol.md):**
```
PacketHeader (4 bytes)
├── magic (2 bytes) = 0x5254 ('RT')
├── type (1 byte)   = JOIN, SPAWN, MOVE, SHOOT, etc.
└── sequence (1 byte) = numéro de séquence
```

**Utilisation dans le jeu:**
```cpp
// Game.cpp - Ligne 440+
if (argc >= 3) {
    std::string serverIp = argv[1];
    int serverPort = std::stoi(argv[2]);
    networkClient.Connect(serverIp, serverPort);
    isNetworkClient = true;
}
```

---

### 5️⃣ Scripting - Intégration Lua

**Localisation:** `engine/include/scripting/`

#### Système de Scripts:

| Fichier | Fonction | Usage |
|---------|----------|-------|
| `LuaState.hpp` | `Init()`, `GetState()` | Initialisation Lua |
| | `EnableHotReload(bool)` | Rechargement à chaud |
| | `ExecuteFile(path)` | Exécution script |
| `ComponentBindings.hpp` | `RegisterAll(lua)` | Liaison composants ECS → Lua |
| | `RegisterCoordinator(lua, coord)` | Accès ECS depuis Lua |
| `ScriptSystem.hpp` | `LoadScript(path)` | Chargement scripts entités |
| | `Update(dt)` | Exécution scripts par frame |
| `PrefabManager.hpp` | `LoadPrefab(name)` | Chargement préfabs Lua |

**Utilisation dans Game.cpp (lignes 483-497):**
```cpp
auto& luaState = Scripting::LuaState::Instance();
luaState.Init();
luaState.EnableHotReload(true);

Scripting::ComponentBindings::RegisterAll(luaState.GetState());
Scripting::ComponentBindings::RegisterCoordinator(luaState.GetState(), &gCoordinator);
```

**Exemple de script Lua (`assets/scripts/`):**
```lua
-- Enemy spawn script
function SpawnWave(coordinator, time)
    local enemy = coordinator:CreateEntity()
    coordinator:AddComponent(enemy, "Position", {x = 1800, y = 500})
    coordinator:AddComponent(enemy, "Velocity", {x = -200, y = 0})
end
```

---

### 6️⃣ Systems - Systèmes Génériques

**Localisation:** `engine/include/systems/`

#### Systèmes Disponibles:

| Système | Fichier | Signature | Fonction |
|---------|---------|-----------|----------|
| **MovementSystem** | `MovementSystem.hpp` | Position + Velocity | Déplace les entités selon leur vélocité |
| **AnimationSystem** | `AnimationSystem.hpp` | Animation + Sprite | Anime les sprites (frame par frame) |
| **StateMachineAnimationSystem** | `StateMachineAnimationSystem.hpp` | StateMachineAnimation + Sprite | Animation avec machine à états |
| **RenderSystem** | `RenderSystem.hpp` | Position + Sprite | Affiche les sprites à l'écran |
| **CollisionSystem** | `CollisionSystem.hpp` | Position + Collider | Détecte les collisions AABB |
| **HealthSystem** | `HealthSystem.hpp` | Health | Gère la santé et la mort |
| **BoundarySystem** | `BoundarySystem.hpp` | Position | Maintient entités dans limites |
| **LifetimeSystem** | `LifetimeSystem.hpp` | Lifetime | Détruit entités après durée |
| **ScrollingBackgroundSystem** | `ScrollingBackgroundSystem.hpp` | ScrollingBackground + Position | Défilement parallaxe |

**Enregistrement dans Game.cpp (lignes 516-640):**
```cpp
// Enregistrement système
movementSystem = gCoordinator.RegisterSystem<MovementSystem>(&gCoordinator);

// Définition signature
ECS::Signature movementSig;
movementSig.set(gCoordinator.GetComponentType<Position>());
movementSig.set(gCoordinator.GetComponentType<Velocity>());
gCoordinator.SetSystemSignature<MovementSystem>(movementSig);

// Initialisation
movementSystem->Init();
```

**Update des systèmes (boucle de jeu):**
```cpp
movementSystem->Update(dt);
animationSystem->Update(dt);
collisionSystem->Update(dt);
renderSystem->Update(dt);
```

---

### 7️⃣ Components - Composants Génériques

**Localisation:** `engine/include/components/`

#### Composants de Base:

| Composant | Fichier | Données | Usage |
|-----------|---------|---------|-------|
| **Position** | `Position.hpp` | `float x, y` | Position 2D dans le monde |
| **Velocity** | `Velocity.hpp` | `float x, y` | Vitesse de déplacement |
| **Sprite** | `Sprite.hpp` | `ISprite* sprite`, `IntRect textureRect`, `int layer` | Rendu visuel |
| **Animation** | `Animation.hpp` | `int frameCount`, `float frameTime`, `float elapsed` | Animation frame-by-frame |
| **Collider** | `Collider.hpp` | `float width, height`, `string tag` | Collision AABB |
| **Health** | `Health.hpp` | `int current, max` | Points de vie |
| **Tag** | `Tag.hpp` | `string name` | Identification entité |
| **Lifetime** | `Lifetime.hpp` | `float duration, elapsed` | Durée de vie limitée |
| **NetworkId** | `NetworkId.hpp` | `uint32_t id` | Synchronisation réseau |
| **Boundary** | `Boundary.hpp` | `float minX, maxX, minY, maxY` | Limites de mouvement |
| **ScrollingBackground** | `ScrollingBackground.hpp` | `float speed`, `float resetX` | Parallaxe |

**Exemple d'utilisation:**
```cpp
// Game.cpp - CreatePlayer() ligne 35+
gCoordinator.AddComponent(player, Position{x, y});
gCoordinator.AddComponent(player, Velocity{0.0f, 0.0f});
gCoordinator.AddComponent(player, Health{100, 100});
gCoordinator.AddComponent(player, Tag{"player"});
```

---

### 8️⃣ Module Shoot'em Up

**Localisation:** `engine/modules/shootemup/`

Ce module étend l'engine avec des fonctionnalités spécifiques aux shoot'em up.

#### Components Shoot'em Up:

| Composant | Fichier | Description |
|-----------|---------|-------------|
| **Weapon** | `Weapon.hpp` | Système d'armes (cadence, charge, type) |
| **MovementPattern** | `MovementPattern.hpp` | Patterns de mouvement ennemis |
| **PowerUp** | `PowerUp.hpp` | Bonus et améliorations |
| **AIController** | `AIController.hpp` | IA ennemis |
| **Attachment** | `Attachment.hpp` | Attachement d'entités (modules, effets) |
| **Effect** | `Effect.hpp` | Effets visuels et sonores |
| **ShootEmUpTags** | `ShootEmUpTags.hpp` | Tags spécifiques (PlayerTag, EnemyTag, ProjectileTag) |

#### Systems Shoot'em Up:

| Système | Fichier | Fonction |
|---------|---------|----------|
| **WeaponSystem** | `WeaponSystem.hpp` | Gestion du tir et de la charge |
| **MovementPatternSystem** | `MovementPatternSystem.hpp` | Application des patterns (sinusoïdal, circulaire, etc.) |
| **EnemySpawnSystem** | `EnemySpawnSystem.hpp` | Génération de vagues d'ennemis |

#### Factories Shoot'em Up:

| Factory | Fichier | Fonction |
|---------|---------|----------|
| **EnemyFactory** | `EnemyFactory.hpp` | Création ennemis préconfigurés |
| **ProjectileFactory** | `ProjectileFactory.hpp` | Création projectiles (missiles, lasers) |

**Exemple d'utilisation:**
```cpp
// Création ennemis avec patterns
auto enemy = enemyFactory.Create("zigzag", Position{1800, 500});

// Système de tir avec charge
Weapon weapon;
weapon.fireRate = 0.2f;
weapon.supportsCharge = true;
weapon.maxChargeTime = 1.0f;
gCoordinator.AddComponent(player, weapon);
```

---

## 🎮 Utilisation par le Jeu

### Structure Game.cpp

**Localisation:** `game/src/Game.cpp` (1525 lignes)

#### Fonctions Principales:

| Fonction | Ligne | Description |
|----------|-------|-------------|
| `Run(argc, argv)` | 420+ | Point d'entrée principal |
| `CreatePlayer(x, y, line)` | 35 | Crée une entité joueur |
| `CreateBackground(x, y, height, first)` | 97 | Crée un fond défilant |
| `CreateEnemy(x, y, pattern)` | 146 | Crée un ennemi |
| `CreateMissile(x, y, charged, level)` | 213 | Crée un missile |
| `CreateExplosion(x, y)` | 284 | Crée une explosion |
| `CreateShootEffect(x, y, parent)` | 338 | Crée un effet de tir |
| `RegisterEntity(entity)` | 3 | Enregistre une entité |
| `DestroyEntityDeferred(entity)` | 7 | Marque pour destruction |
| `ProcessDestroyedEntities()` | 11 | Nettoie les entités détruites |

#### Flux d'Exécution de Run():

```
1. Parse arguments (réseau ou local)
   ├── --network <ip> <port> → mode client réseau
   └── sinon → mode local

2. Initialisation ECS
   └── gCoordinator.Init()

3. Enregistrement Composants (lignes 460-478)
   ├── Position, Velocity, Sprite, Animation
   ├── Collider, Health, Tag, NetworkId
   └── Weapon, MovementPattern, PowerUp...

4. Initialisation Lua (lignes 483-497)
   ├── LuaState::Init()
   ├── ComponentBindings::RegisterAll()
   └── EnableHotReload()

5. Enregistrement Systèmes (lignes 516-640)
   ├── MovementSystem, AnimationSystem
   ├── CollisionSystem, HealthSystem
   ├── RenderSystem, NetworkSystem
   └── WeaponSystem, MovementPatternSystem

6. Chargement Ressources (lignes 700+)
   ├── Textures (player, enemy, missiles, explosions)
   ├── Sons (shoot, explosion)
   └── Scripts Lua (spawn, patterns)

7. Création Fenêtre et Renderer
   └── SFMLWindow + SFMLRenderer

8. Création Entités Initiales
   ├── CreateBackground() × 2
   └── CreatePlayer() si mode local

9. Boucle Principale (lignes 900+)
   ├── Gestion événements (input, fenêtre)
   ├── Update systèmes (dt)
   │   ├── MovementSystem
   │   ├── AnimationSystem
   │   ├── CollisionSystem
   │   ├── WeaponSystem
   │   ├── HealthSystem
   │   └── NetworkSystem (si mode réseau)
   ├── Rendu (RenderSystem)
   └── ProcessDestroyedEntities()

10. Nettoyage
    └── Destruction textures, sprites, entités
```

---

## 🔄 Flux de Données

### 1. Création d'une Entité Joueur

```
Game::CreatePlayer(x, y)
    │
    ├─→ gCoordinator.CreateEntity()
    │       └─→ EntityManager::CreateEntity()
    │               └─→ Retourne Entity ID
    │
    ├─→ gCoordinator.AddComponent<Position>(entity, {x, y})
    │       └─→ ComponentManager::AddComponent()
    │
    ├─→ gCoordinator.AddComponent<Velocity>(entity, {0, 0})
    │
    ├─→ Création SFMLSprite
    │       ├─→ setTexture(playerTexture)
    │       ├─→ setTextureRect(rect)
    │       └─→ setPosition(x, y)
    │
    ├─→ gCoordinator.AddComponent<Sprite>(entity, sprite)
    │
    ├─→ gCoordinator.AddComponent<Collider>(entity, {width, height, "player"})
    │
    ├─→ gCoordinator.AddComponent<Health>(entity, {100, 100})
    │
    └─→ gCoordinator.AddComponent<Weapon>(entity, weapon_config)
```

### 2. Update Frame (60 FPS)

```
Boucle Principale
    │
    ├─→ PollEvents() → InputSystem
    │       └─→ Met à jour Velocity selon inputs
    │
    ├─→ Update Systèmes (dt = 0.016s)
    │   │
    │   ├─→ MovementSystem::Update(dt)
    │   │       └─→ Position += Velocity * dt
    │   │
    │   ├─→ AnimationSystem::Update(dt)
    │   │       └─→ Avance frame animation
    │   │
    │   ├─→ CollisionSystem::Update(dt)
    │   │       ├─→ Détecte collisions AABB
    │   │       └─→ Callback collision
    │   │               ├─→ CreateExplosion()
    │   │               └─→ Damage entities
    │   │
    │   ├─→ WeaponSystem::Update(dt)
    │   │       ├─→ Gère cooldown tir
    │   │       ├─→ Gère charge missile
    │   │       └─→ CreateMissile() si tir
    │   │
    │   ├─→ MovementPatternSystem::Update(dt)
    │   │       └─→ Applique patterns ennemis
    │   │
    │   ├─→ HealthSystem::Update(dt)
    │   │       └─→ DestroyEntityDeferred() si HP ≤ 0
    │   │
    │   └─→ LifetimeSystem::Update(dt)
    │           └─→ DestroyEntityDeferred() si expiré
    │
    ├─→ RenderSystem::Update(dt)
    │       ├─→ Trie sprites par layer
    │       └─→ Renderer::Draw(sprite)
    │
    └─→ ProcessDestroyedEntities()
            └─→ Nettoie sprites et entités
```

### 3. Mode Réseau - Synchronisation

```
Client                          Serveur
  │                                │
  ├─→ Connect(ip, port)            │
  │         └──────────────────→   │
  │                                ├─→ Accepte connexion
  │                                ├─→ Envoie état initial
  │   ←──────────────────────────  │   (SPAWN packets)
  │                                │
  ├─→ Input détecté                │
  ├─→ SendInputPacket()            │
  │         └──────────────────→   │
  │                                ├─→ Applique input
  │                                ├─→ Update simulation
  │                                └─→ Broadcast état
  │   ←──────────────────────────  │   (MOVE, SHOOT packets)
  │                                │
  ├─→ NetworkSystem::Update()      │
  │   ├─→ Receive packets          │
  │   ├─→ Apply state updates      │
  │   └─→ Update entities           │
  │                                │
```

**Packets du Protocole:**
- `JOIN` (0x01): Connexion client
- `SPAWN` (0x02): Création entité
- `MOVE` (0x03): Mise à jour position
- `SHOOT` (0x04): Tir projectile
- `DESTROY` (0x05): Destruction entité
- `STATE` (0x06): État complet du jeu

---

## 📊 Diagrammes Détaillés

### Architecture ECS

```
┌────────────────────────────────────────────────┐
│            ECS::Coordinator                    │
├────────────────────────────────────────────────┤
│  - EntityManager                               │
│  - ComponentManager                            │
│  - SystemManager                               │
├────────────────────────────────────────────────┤
│  + Init()                                      │
│  + CreateEntity() → Entity                     │
│  + DestroyEntity(Entity)                       │
│  + RegisterComponent<T>()                      │
│  + AddComponent<T>(Entity, T)                  │
│  + GetComponent<T>(Entity) → T&                │
│  + HasComponent<T>(Entity) → bool              │
│  + RemoveComponent<T>(Entity)                  │
│  + RegisterSystem<T>() → shared_ptr<T>         │
│  + SetSystemSignature<T>(Signature)            │
└────────────────────────────────────────────────┘
           │               │              │
           ▼               ▼              ▼
    ┌──────────┐   ┌─────────────┐   ┌──────────┐
    │ Entities │   │ Components  │   │ Systems  │
    ├──────────┤   ├─────────────┤   ├──────────┤
    │ Entity 0 │   │ Position[]  │   │Movement  │
    │ Entity 1 │   │ Velocity[]  │   │Animation │
    │ Entity 2 │   │ Sprite[]    │   │Collision │
    │   ...    │   │ Health[]    │   │Render    │
    └──────────┘   └─────────────┘   └──────────┘
```

### Cycle de Vie d'une Entité

```
┌─────────────────┐
│  CreateEntity   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Add Components  │ ← Position, Velocity, Sprite, Health...
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│  RegisterEntity │ ← Ajoute à allEntities
└────────┬────────┘
         │
         ▼
┌─────────────────────────────────────┐
│      Boucle de Jeu (chaque frame)   │
│  ┌───────────────────────────────┐  │
│  │ Systèmes modifient composants │  │
│  │  - MovementSystem             │  │
│  │  - AnimationSystem            │  │
│  │  - CollisionSystem            │  │
│  │  - ...                        │  │
│  └───────────────────────────────┘  │
└────────┬────────────────────────────┘
         │
         ▼
┌─────────────────┐
│ Condition mort? │ ← Health ≤ 0, Lifetime expiré, Collision
└────┬───────┬────┘
     │ NON   │ OUI
     │       ▼
     │  ┌────────────────────┐
     │  │DestroyDeferred()   │
     │  └────────┬───────────┘
     │           │
     └───────────┤
                 ▼
         ┌───────────────────┐
         │ProcessDestroyed() │
         │ - Delete sprite   │
         │ - DestroyEntity   │
         │ - Remove from list│
         └───────────────────┘
```

---

## 🎯 Composants Utilisés par le Jeu

### Entité Joueur

```cpp
ECS::Entity player = CreatePlayer(100, 540);
```

| Composant | Valeurs | Utilité |
|-----------|---------|---------|
| Position | {100, 540} | Position initiale |
| Velocity | {0, 0} | Contrôlé par InputSystem |
| Sprite | texture: playerTexture, rect: {66, 0, 33, 17} | Visuel joueur |
| StateMachineAnimation | currentColumn: 2, targetColumn: 2 | Animation idle/accélération |
| Collider | {99, 51, "player"} | Détection collision |
| Health | {100, 100} | Points de vie |
| Weapon | fireRate: 0.2, supportsCharge: true | Tir missiles |
| Tag | "player" | Identification |
| PlayerTag | playerId: 0 | Tag spécifique joueur |

### Entité Ennemi

```cpp
ECS::Entity enemy = CreateEnemy(1800, 500, "zigzag");
```

| Composant | Valeurs | Utilité |
|-----------|---------|---------|
| Position | {1800, 500} | Hors écran à droite |
| Velocity | {-200, 0} | Déplace vers la gauche |
| Sprite | texture: enemyTexture | Visuel ennemi |
| Animation | frameCount: 2 | Animation spritesheet |
| Collider | {99, 75, "enemy"} | Détection collision |
| Health | {50, 50} | Points de vie |
| MovementPattern | type: "zigzag", amplitude: 100 | Pattern de mouvement |
| Tag | "enemy" | Identification |
| EnemyTag | {} | Tag spécifique ennemi |

### Entité Missile

```cpp
ECS::Entity missile = CreateMissile(playerX, playerY, true, 3);
```

| Composant | Valeurs | Utilité |
|-----------|---------|---------|
| Position | {playerX, playerY} | Position de spawn |
| Velocity | {1000, 0} | Vitesse vers la droite |
| Sprite | texture: missileTexture | Visuel missile |
| Collider | {32, 14, "projectile"} | Détection collision |
| Damage | damage: 25 (chargé: 75) | Dégâts infligés |
| Lifetime | 3.0s | Auto-destruction après 3s |
| Tag | "missile" | Identification |
| ProjectileTag | ownerId: playerId | Propriétaire du projectile |

### Entité Explosion

```cpp
ECS::Entity explosion = CreateExplosion(x, y);
```

| Composant | Valeurs | Utilité |
|-----------|---------|---------|
| Position | {x, y} | Position collision |
| Sprite | texture: explosionTexture | Visuel explosion |
| Animation | frameCount: 5, frameTime: 0.1 | Animation explosion |
| Lifetime | 0.5s | Disparaît après animation |
| Effect | type: "explosion" | Effet visuel |

---

## 🎨 Systèmes Détaillés

### MovementSystem

**Fichier:** `engine/src/systems/MovementSystem.cpp`

**Signature:** Position + Velocity

**Algorithme:**
```cpp
void Update(float dt) {
    for (auto entity : entities) {
        auto& pos = GetComponent<Position>(entity);
        auto& vel = GetComponent<Velocity>(entity);
        
        pos.x += vel.x * dt;
        pos.y += vel.y * dt;
    }
}
```

**Usage:** Appliqué à tous les joueurs, ennemis, missiles

---

### CollisionSystem

**Fichier:** `engine/src/systems/CollisionSystem.cpp`

**Signature:** Position + Collider

**Algorithme:**
```cpp
void Update(float dt) {
    for (auto entityA : entities) {
        for (auto entityB : entities) {
            if (entityA >= entityB) continue;
            
            if (CheckAABBCollision(entityA, entityB)) {
                OnCollision(entityA, entityB);
            }
        }
    }
}

bool CheckAABBCollision(Entity a, Entity b) {
    auto& posA = GetComponent<Position>(a);
    auto& collA = GetComponent<Collider>(a);
    auto& posB = GetComponent<Position>(b);
    auto& collB = GetComponent<Collider>(b);
    
    return (posA.x < posB.x + collB.width &&
            posA.x + collA.width > posB.x &&
            posA.y < posB.y + collB.height &&
            posA.y + collA.height > posB.y);
}
```

**Callback (Game.cpp ligne 640+):**
```cpp
collisionSystem->SetCollisionCallback([this](Entity a, Entity b) {
    // Créer explosion
    CreateExplosion(x, y);
    
    // Infliger dégâts
    if (HasComponent<Health>(a)) {
        auto& health = GetComponent<Health>(a);
        health.current -= damage;
    }
    
    // Détruire projectile
    DestroyEntityDeferred(projectile);
});
```

---

### WeaponSystem

**Fichier:** `engine/modules/shootemup/src/systems/WeaponSystem.cpp`

**Signature:** Weapon + Position

**Fonctionnalités:**
- Cooldown de tir (fireRate)
- Charge de missile (chargeTime)
- Création de projectiles

**Algorithme:**
```cpp
void Update(float dt) {
    for (auto entity : entities) {
        auto& weapon = GetComponent<Weapon>(entity);
        auto& pos = GetComponent<Position>(entity);
        
        // Cooldown
        weapon.timeSinceLastShot += dt;
        
        // Input tir
        if (Input::IsKeyPressed(Space)) {
            if (weapon.supportsCharge) {
                weapon.chargeTime += dt;
                weapon.isCharging = true;
            }
        }
        
        // Relâchement
        if (Input::IsKeyReleased(Space) && weapon.isCharging) {
            if (weapon.timeSinceLastShot >= weapon.fireRate) {
                Shoot(entity, weapon.chargeTime);
                weapon.timeSinceLastShot = 0;
                weapon.chargeTime = 0;
                weapon.isCharging = false;
            }
        }
    }
}

void Shoot(Entity owner, float chargeTime) {
    bool isCharged = (chargeTime >= weapon.maxChargeTime);
    int chargeLevel = (int)(chargeTime / weapon.maxChargeTime * 3);
    
    CreateMissile(pos.x, pos.y, isCharged, chargeLevel);
    PlaySound(weapon.shootSound);
}
```

---

### RenderSystem

**Fichier:** `engine/src/systems/RenderSystem.cpp`

**Signature:** Position + Sprite

**Algorithme:**
```cpp
void Update(float dt) {
    // Tri par layer (background → foreground)
    std::sort(entities.begin(), entities.end(), [](Entity a, Entity b) {
        auto& spriteA = GetComponent<Sprite>(a);
        auto& spriteB = GetComponent<Sprite>(b);
        return spriteA.layer < spriteB.layer;
    });
    
    // Rendu
    renderer->Clear();
    for (auto entity : entities) {
        auto& pos = GetComponent<Position>(entity);
        auto& sprite = GetComponent<Sprite>(entity);
        
        sprite.sprite->setPosition(pos.x, pos.y);
        renderer->Draw(sprite.sprite);
    }
    renderer->Present();
}
```

**Layers:**
- 0: Background far
- 5: Background near
- 10: Joueur
- 15: Ennemis
- 20: Projectiles
- 25: Effets/Explosions

---

## 🌐 Protocole Réseau Détaillé

### Types de Packets

```cpp
enum class PacketType : uint8_t {
    JOIN = 0x01,        // Client → Server: Demande connexion
    JOIN_ACK = 0x02,    // Server → Client: Accepte connexion
    SPAWN = 0x03,       // Server → Clients: Crée entité
    MOVE = 0x04,        // Bidirectionnel: Mise à jour position
    SHOOT = 0x05,       // Client → Server: Tir
    DAMAGE = 0x06,      // Server → Clients: Dégâts infligés
    DESTROY = 0x07,     // Server → Clients: Détruit entité
    STATE = 0x08,       // Server → Clients: État complet
    PING = 0x09,        // Bidirectionnel: Keep-alive
    DISCONNECT = 0x0A   // Client → Server: Déconnexion
};
```

### Structure des Packets

#### JOIN Packet
```
[Header: 4 bytes]
├── magic: 0x5254 (2 bytes)
├── type: JOIN (1 byte)
└── sequence: 0 (1 byte)

[Payload: 64 bytes]
└── playerName: char[64]
```

#### SPAWN Packet
```
[Header: 4 bytes]
[Payload: 21 bytes]
├── entityId: uint32_t (4 bytes)
├── entityType: uint8_t (1 byte)  // 0=Player, 1=Enemy, 2=Projectile
├── x: float (4 bytes)
├── y: float (4 bytes)
├── velocityX: float (4 bytes)
└── velocityY: float (4 bytes)
```

#### MOVE Packet
```
[Header: 4 bytes]
[Payload: 20 bytes]
├── entityId: uint32_t (4 bytes)
├── x: float (4 bytes)
├── y: float (4 bytes)
├── velocityX: float (4 bytes)
└── velocityY: float (4 bytes)
```

### Gestion de la Latence

**Client-Side Prediction:**
```cpp
// Client applique immédiatement l'input
OnInput() {
    ApplyInputLocally();
    SendInputToServer();
}

// Puis corrige avec état serveur
OnServerState() {
    if (abs(serverPos - clientPos) > THRESHOLD) {
        SmoothCorrection(serverPos);
    }
}
```

**Server Reconciliation:**
```cpp
// Serveur autoritaire pour collisions/dégâts
OnClientInput(playerId, input, timestamp) {
    ReplayInputsSince(timestamp);
    BroadcastState(excludePlayer: playerId);
}
```

---

## 📝 Scripts Lua - Exemples

### Configuration Vague d'Ennemis

**Fichier:** `assets/scripts/waves/wave1.lua`

```lua
-- Wave 1: Formation simple
function SpawnWave(coordinator, elapsed)
    if elapsed < 5 then
        return  -- Attendre 5 secondes
    end
    
    -- Spawn 3 ennemis en formation
    for i = 0, 2 do
        local enemy = coordinator:CreateEntity()
        
        coordinator:AddComponent(enemy, "Position", {
            x = 1920,
            y = 200 + i * 250
        })
        
        coordinator:AddComponent(enemy, "Velocity", {
            x = -150,
            y = 0
        })
        
        coordinator:AddComponent(enemy, "MovementPattern", {
            type = "sine",
            amplitude = 100,
            frequency = 2.0
        })
        
        coordinator:AddComponent(enemy, "Health", {
            current = 50,
            max = 50
        })
        
        coordinator:AddComponent(enemy, "EnemyTag", {})
    end
end
```

### Comportement Ennemi Boss

**Fichier:** `assets/scripts/entities/boss.lua`

```lua
-- Boss avec phases
local phase = 1
local health_threshold_phase2 = 0.5
local health_threshold_phase3 = 0.25

function Update(entity, dt, coordinator)
    local health = coordinator:GetComponent(entity, "Health")
    local weapon = coordinator:GetComponent(entity, "Weapon")
    
    -- Phase 1: Tir normal
    if health.current / health.max > health_threshold_phase2 then
        weapon.fireRate = 1.0
        weapon.projectileCount = 1
    
    -- Phase 2: Tir rapide
    elseif health.current / health.max > health_threshold_phase3 then
        weapon.fireRate = 0.3
        weapon.projectileCount = 3
    
    -- Phase 3: Pattern complexe
    else
        weapon.fireRate = 0.5
        weapon.projectileCount = 5
        weapon.spreadAngle = 45
    end
end
```

---

## 🛠️ Compilation et Exécution

### Prérequis

- CMake ≥ 3.15
- C++20
- SFML 2.6
- Lua 5.4 (optionnel, pour scripting)
- ASIO (inclus via CPM)

### Compilation

```bash
cd /home/zeroualwassim/3emeAnnee/Game/rtype
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### Exécution

**Mode Local:**
```bash
./build/game/r-type_game
```

**Mode Serveur:**
```bash
./build/server/r-type_server 8080
```

**Mode Client Réseau:**
```bash
./build/game/r-type_game --network 127.0.0.1 8080
```

---

## 📂 Fichiers Importants

### Configuration

| Fichier | Description |
|---------|-------------|
| `CMakeLists.txt` | Configuration CMake racine |
| `engine/CMakeLists.txt` | Build de l'engine |
| `game/CMakeLists.txt` | Build du jeu |
| `docker-compose.yml` | Déploiement Docker |

### Documentation

| Fichier | Contenu |
|---------|---------|
| `Protocol.md` | Protocole réseau UDP |
| `docs/ENGINE_IMPLEMENTATION_GUIDE.md` | Guide implémentation |
| `docs/MODULE_ARCHITECTURE.md` | Architecture modules |
| `REFACTORING_SUMMARY.md` | Historique refactoring |

### Assets

| Dossier | Contenu |
|---------|---------|
| `assets/enemies/` | Sprites ennemis |
| `assets/players/` | Sprites joueurs |
| `assets/vfx/` | Effets visuels |
| `assets/scripts/` | Scripts Lua |

---

## 🔍 Points d'Extension

### Ajouter un Nouveau Composant

1. **Créer le header** `engine/include/components/NewComponent.hpp`
```cpp
struct NewComponent {
    float value;
    bool enabled;
};
```

2. **Enregistrer dans Game.cpp**
```cpp
gCoordinator.RegisterComponent<NewComponent>();
```

3. **Créer un système** qui l'utilise
```cpp
class NewSystem : public ECS::System {
public:
    void Update(float dt) override {
        for (auto entity : entities) {
            auto& comp = GetComponent<NewComponent>(entity);
            // Logique...
        }
    }
};
```

### Ajouter un Nouveau Système

1. **Créer** `engine/include/systems/NewSystem.hpp`
2. **Implémenter** dans `engine/src/systems/NewSystem.cpp`
3. **Enregistrer dans Game.cpp:**
```cpp
auto newSystem = gCoordinator.RegisterSystem<NewSystem>();
ECS::Signature sig;
sig.set(gCoordinator.GetComponentType<RequiredComponent>());
gCoordinator.SetSystemSignature<NewSystem>(sig);
newSystem->Init();
```

4. **Appeler Update()** dans la boucle de jeu

### Ajouter un Nouveau Type de Packet Réseau

1. **Définir dans** `engine/include/network/Protocol.hpp`
```cpp
enum class PacketType : uint8_t {
    // ... existants
    NEW_PACKET = 0x0B
};
```

2. **Créer structure** dans `Packet.hpp`
```cpp
struct NewPacketData {
    uint32_t field1;
    float field2;
};
```

3. **Gérer côté serveur** dans `NetworkServer.cpp`
4. **Gérer côté client** dans `NetworkClient.cpp`

---

## 📊 Statistiques du Projet

- **Lignes de code Engine:** ~15,000
- **Lignes de code Game:** ~1,500
- **Nombre de composants:** 20+
- **Nombre de systèmes:** 15+
- **Fichiers headers:** 73+
- **Modules:** 2 (Core + ShootEmUp)

---

## 🎓 Concepts Avancés Utilisés

### 1. Entity Component System (ECS)
- **Data-Oriented Design**
- **Cache-Friendly** (composants stockés en tableaux contigus)
- **Découplage total** logique/données

### 2. Dependency Injection
- Systèmes reçoivent Coordinator via constructeur
- Permet le testing et le découplage

### 3. Observer Pattern
- EventBus pour communication entre systèmes
- Collisions notifiées via callbacks

### 4. Factory Pattern
- EnemyFactory, ProjectileFactory
- Création d'entités préconfigurées

### 5. State Machine
- StateMachineAnimationSystem
- Transitions fluides entre états

### 6. Object Pool Pattern
- Réutilisation des entités détruites (évite allocations)

### 7. Hot Reload
- Scripts Lua rechargés à la volée
- Pas besoin de recompiler

---

## 🚀 Fonctionnalités Implémentées

✅ **ECS complet** (Entities, Components, Systems)  
✅ **Rendering abstrait** (SFML actuellement, extensible)  
✅ **Animation** (frame-by-frame + state machine)  
✅ **Collision AABB** avec callbacks  
✅ **Système de santé et dégâts**  
✅ **Système d'armes** (tir, charge, cooldown)  
✅ **Patterns de mouvement** ennemis  
✅ **Parallax scrolling** backgrounds  
✅ **Scripting Lua** avec hot-reload  
✅ **Réseau UDP** client-serveur  
✅ **Protocole binaire** optimisé  
✅ **Factories** pour création d'entités  
✅ **Système de lifetime** (auto-destruction)  
✅ **Effets visuels** (explosions, tirs)  

---

## 📖 Ressources Complémentaires

- **Docs officielles:** [docs/](../docs/)
- **Protocol:** [Protocol.md](../Protocol.md)
- **Guide d'implémentation:** [docs/ENGINE_IMPLEMENTATION_GUIDE.md](../docs/ENGINE_IMPLEMENTATION_GUIDE.md)
- **Architecture modules:** [docs/MODULE_ARCHITECTURE.md](../docs/MODULE_ARCHITECTURE.md)

---

## 🎯 Résumé

L'**Engine R-Type** est un moteur de jeu modulaire basé sur l'architecture **ECS**, conçu pour être:

- **Extensible:** Ajout facile de composants/systèmes
- **Performant:** Data-Oriented Design, cache-friendly
- **Flexible:** Scripting Lua, rendering abstrait
- **Networked:** Communication UDP avec protocole optimisé
- **Maintenable:** Code découplé, documentation complète

Le **Jeu R-Type** utilise toutes ces fonctionnalités pour créer un shoot'em up multijoueur avec:
- Joueurs avec armes chargées
- Ennemis avec patterns de mouvement
- Système de collision et dégâts
- Effets visuels et sonores
- Mode local et multijoueur réseau

---

*Document créé le 13 janvier 2026 - R-Type Project*
