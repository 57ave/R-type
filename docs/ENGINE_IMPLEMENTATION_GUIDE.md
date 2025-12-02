# Game Engine Implementation Guide - R-Type Project

## 📋 Table des Matières
1. [Vue d'ensemble](#vue-densemble)
2. [Architecture ECS (Entity Component System)](#architecture-ecs)
3. [Subsystems à implémenter](#subsystems-à-implémenter)
4. [Abstractions nécessaires](#abstractions-nécessaires)
5. [Plan d'implémentation](#plan-dimplémentation)
6. [Intégration avec Client/Server](#intégration-avec-clientserver)

---

## 🎯 Vue d'ensemble

Le **Game Engine** est la couche centrale qui fournit les fonctionnalités réutilisables pour le client et le serveur. Il doit être **agnostique** du jeu R-Type lui-même.

### Objectifs principaux
- ✅ Architecture ECS (Entity Component System)
- ✅ Subsystem de rendu (client uniquement)
- ✅ Subsystem de networking (abstraction)
- ✅ Subsystem de logique de jeu
- ✅ Indépendant de la plateforme
- ✅ Réutilisable et extensible

### Structure des dossiers
```
engine/
├── include/engine/
│   ├── core/           # Core engine features
│   ├── ecs/            # Entity Component System
│   └── network/        # Network abstractions
└── src/                # Implementations
```

---

## 🏗️ Architecture ECS (Entity Component System)

### Pourquoi ECS ?
L'ECS sépare les **données** (Components) de la **logique** (Systems) et des **identités** (Entities). Cela permet :
- 🔄 Flexibilité maximale
- ⚡ Performance (cache-friendly)
- 🧩 Composition over inheritance
- 🔧 Facile à étendre

### Composants de l'ECS

#### 1. **Entity**
```
Concept : Un simple ID unique (uint32_t ou uint64_t)
Rôle    : Identifiant pour grouper des components
Exemple : Entity player = 42;
```

**À implémenter :**
- `EntityManager` : Création, destruction, recyclage d'IDs
- Gestion du cycle de vie des entities
- Mapping entity → components

#### 2. **Component**
```
Concept : Structures de données pures (POD - Plain Old Data)
Rôle    : Stockent l'état (position, vitesse, sprite, etc.)
Exemple : struct Position { float x, y; };
```

**À implémenter :**
- `ComponentManager<T>` : Container générique pour chaque type de component
- Storage dense (SparseSet ou PackedArray)
- Accès O(1) par entity
- Itération rapide sur tous les components d'un type

**Components typiques pour R-Type :**
```cpp
// Transform
struct Position { float x, y; };
struct Velocity { float vx, vy; };
struct Rotation { float angle; };
struct Scale { float sx, sy; };

// Rendering
struct Sprite { 
    std::string texturePath;
    IntRect textureRect;
    int layer;
};
struct Animator {
    std::vector<IntRect> frames;
    float frameTime;
    size_t currentFrame;
};

// Physics
struct Collider {
    float width, height;
    bool isTrigger;
};
struct RigidBody {
    float mass;
    bool useGravity;
};

// Gameplay
struct Health { int current, max; };
struct Damage { int amount; };
struct PlayerInput { /* ... */ };
struct AIController { /* ... */ };

// Network
struct NetworkId { uint32_t id; };
struct Replicated { /* ... */ };
```

#### 3. **System**
```
Concept : Logique pure qui opère sur des components
Rôle    : Comportement du jeu (mouvement, collision, rendu)
Exemple : MovementSystem lit Position + Velocity, met à jour Position
```

**À implémenter :**
- Interface `ISystem` avec `update(deltaTime)`
- `SystemManager` : Orchestre l'ordre d'exécution
- Query system pour filtrer entities avec des components spécifiques

**Systems typiques pour R-Type :**
```cpp
// Core systems
- MovementSystem         : Position + Velocity → update position
- CollisionSystem        : Collider + Position → detect & resolve
- AnimationSystem        : Animator + Sprite → update frames
- LifetimeSystem         : TTL component → destroy entities

// Rendering (client only)
- RenderSystem           : Sprite + Position → draw
- ParticleSystem         : Particle emitters
- CameraSystem           : Camera follow, bounds

// Gameplay
- InputSystem            : Read input → PlayerInput component
- WeaponSystem           : Fire weapons, spawn bullets
- HealthSystem           : Damage events → Health update
- AISystem               : AI logic for enemies

// Network (client/server)
- NetworkSyncSystem      : Replicate entities over network
- PredictionSystem       : Client-side prediction
- ReconciliationSystem   : Correct mispredictions
```

#### 4. **Registry/World**
```
Concept : Container central qui gère entities, components, systems
Rôle    : Point d'entrée pour toutes les opérations ECS
```

**À implémenter :**
```cpp
class Registry {
public:
    // Entity management
    Entity createEntity();
    void destroyEntity(Entity e);
    bool isValid(Entity e);
    
    // Component management
    template<typename T>
    T& addComponent(Entity e, T&& component);
    
    template<typename T>
    T& getComponent(Entity e);
    
    template<typename T>
    bool hasComponent(Entity e);
    
    template<typename T>
    void removeComponent(Entity e);
    
    // Query entities
    template<typename... Components>
    View<Components...> view();
    
    // System management
    template<typename T, typename... Args>
    T& addSystem(Args&&... args);
    
    void update(float deltaTime);
};
```

---

## 🧱 Subsystems à implémenter

### 1. **Core Subsystem** (`engine/core/`)

#### a) **Time Management**
```cpp
class Time {
    - deltaTime : float        // Temps écoulé depuis la dernière frame
    - totalTime : float        // Temps total depuis le démarrage
    - timeScale : float        // Pour ralenti/accéléré
    
    + getDeltaTime() → float
    + getTotalTime() → float
    + setTimeScale(float)
};
```

#### b) **Event System**
```cpp
class EventBus {
    // Pub/Sub pattern pour communication décuplée
    + subscribe<T>(callback)
    + publish<T>(event)
    + unsubscribe(handle)
};

// Exemples d'events
struct EntityDestroyedEvent { Entity entity; };
struct CollisionEvent { Entity a, b; };
struct InputEvent { /* ... */ };
```

#### c) **Resource Manager**
```cpp
template<typename T>
class ResourceManager {
    // Cache de ressources (textures, sons, etc.)
    + load(path) → shared_ptr<T>
    + unload(path)
    + get(path) → shared_ptr<T>
    
private:
    std::unordered_map<std::string, std::shared_ptr<T>> resources;
};
```

#### d) **Logger**
```cpp
class Logger {
    + info(message)
    + warning(message)
    + error(message)
    + debug(message)
    
    // Avec support pour différents outputs
};
```

#### e) **Configuration**
```cpp
class Config {
    // Lecture de fichiers de configuration (JSON, TOML, etc.)
    + load(path)
    + get<T>(key) → T
    + set<T>(key, value)
};
```

---

### 2. **Rendering Subsystem** (`engine/rendering/` ou dans `core/`)

⚠️ **Note** : Utilisé **uniquement par le client**, pas le serveur

#### a) **Graphics Abstraction**
```cpp
// Interface pour abstraire SFML/SDL/autre
class IRenderer {
public:
    virtual void clear() = 0;
    virtual void draw(Sprite, Transform) = 0;
    virtual void present() = 0;
};

class SFMLRenderer : public IRenderer { /* ... */ };
```

#### b) **Window Management**
```cpp
class Window {
    + create(width, height, title)
    + isOpen() → bool
    + pollEvents() → vector<Event>
    + close()
    + getSize() → Vector2u
};
```

#### c) **Camera**
```cpp
class Camera {
    Position position;
    float zoom;
    IntRect viewport;
    
    + worldToScreen(Vector2f) → Vector2f
    + screenToWorld(Vector2f) → Vector2f
};
```

#### d) **Layer/Rendering Order**
```cpp
// Pour gérer l'ordre de rendu (background → players → effects)
class RenderLayer {
    int layer;
    vector<RenderCommand> commands;
};
```

---

### 3. **Network Subsystem** (`engine/network/`)

⚠️ **Important** : Doit être une **abstraction** utilisable par client ET serveur

#### a) **Packet System**
```cpp
// Sérialisation/désérialisation de données réseau
class Packet {
    + write<T>(value)
    + read<T>() → T
    + getData() → byte[]
    + getSize() → size_t
};

// Exemples de packets
struct PlayerMovePacket {
    uint32_t playerId;
    float x, y;
    float vx, vy;
};

struct SpawnEntityPacket {
    uint32_t entityId;
    EntityType type;
    float x, y;
};

struct DestroyEntityPacket {
    uint32_t entityId;
};
```

#### b) **Network Interface**
```cpp
// Abstraction socket (UDP/TCP)
class INetworkSocket {
public:
    virtual void send(Packet, Address) = 0;
    virtual optional<Packet> receive() = 0;
    virtual void bind(port) = 0;
};

class UDPSocket : public INetworkSocket { /* ... */ };
class TCPSocket : public INetworkSocket { /* ... */ };
```

#### c) **Connection Management**
```cpp
class Connection {
    Address remoteAddress;
    ConnectionState state;
    float lastPingTime;
    
    + send(Packet)
    + isConnected() → bool
    + disconnect()
};

class ConnectionManager {
    + addConnection(Address) → Connection&
    + removeConnection(Address)
    + getConnection(Address) → Connection*
    + broadcastToAll(Packet)
};
```

#### d) **Interpolation & Prediction** (Client-side)
```cpp
// Pour smooth movement malgré latency
class NetworkInterpolator {
    + addSnapshot(timestamp, state)
    + interpolate(currentTime) → state
};

// Client-side prediction
class ClientPredictor {
    + predictMovement(input, deltaTime)
    + reconcile(serverState)
};
```

---

### 4. **Physics Subsystem** (`engine/physics/` ou dans `core/`)

#### a) **Collision Detection**
```cpp
class CollisionDetector {
    + checkAABB(Box a, Box b) → bool
    + checkCircle(Circle a, Circle b) → bool
    + checkRaycast(Ray, Collider) → optional<Hit>
};

struct CollisionInfo {
    Entity a, b;
    Vector2f normal;
    float penetration;
};
```

#### b) **Spatial Partitioning** (optionnel, pour optimisation)
```cpp
class QuadTree {
    // Pour éviter de tester toutes les collisions
    + insert(Entity, Bounds)
    + query(Bounds) → vector<Entity>
};
```

---

### 5. **Input Subsystem** (`engine/input/`)

```cpp
class InputManager {
    + isKeyPressed(Key) → bool
    + isKeyJustPressed(Key) → bool
    + isKeyReleased(Key) → bool
    + getMousePosition() → Vector2i
    
    // Support gamepad
    + isButtonPressed(Button, gamepadId) → bool
};
```

---

## 🎨 Abstractions nécessaires

### Pourquoi abstraire ?
- ✅ Indépendance de la plateforme
- ✅ Testabilité (mock objects)
- ✅ Flexibilité (changer de lib sans tout casser)

### Abstractions clés

#### 1. **Graphics (SFML → Abstraction)**
```
ITexture, ISprite, IRenderer, IWindow
→ Permet de switcher SFML pour SDL ou autre
```

#### 2. **Network (Asio → Abstraction)**
```
ISocket, IPacket, IConnection
→ Permet d'utiliser raw sockets ou autre lib
```

#### 3. **Audio (optionnel)**
```
IAudioSource, IAudioListener
```

#### 4. **Math Library**
```
Vector2, Vector3, Matrix, Quaternion
→ Peut utiliser GLM ou créer la vôtre
```

---

## 📅 Plan d'implémentation

### 🔵 Phase 1 : Fondations ECS (Semaine 1-2)

**Priorité CRITIQUE**

1. **Entity Manager**
   - Génération d'IDs uniques
   - Recyclage d'entities détruites
   - Version/génération pour détecter stale entities

2. **Component Storage**
   - `SparseSet<T>` ou `ComponentArray<T>`
   - Ajout/suppression/récupération de components

3. **Registry de base**
   - `createEntity()`, `destroyEntity()`
   - `addComponent<T>()`, `getComponent<T>()`
   - `hasComponent<T>()`, `removeComponent<T>()`

4. **View/Query System**
   - `view<Position, Velocity>()` pour itérer
   - Support pour 1 à N components

**Validation** : Pouvoir créer des entities avec components et itérer dessus

---

### 🟢 Phase 2 : Systems de base (Semaine 2-3)

1. **Interface ISystem**
   ```cpp
   class ISystem {
   public:
       virtual void update(Registry&, float dt) = 0;
   };
   ```

2. **MovementSystem**
   - Lit Position + Velocity
   - Met à jour Position

3. **SystemManager**
   - Enregistre systems
   - Appelle `update()` dans le bon ordre

4. **Time Management**
   - DeltaTime calculation
   - Fixed timestep (optionnel)

**Validation** : Entities se déplacent correctement

---

### 🟡 Phase 3 : Rendering (Semaine 3)

**Client uniquement**

1. **Graphics Abstraction**
   - `IRenderer`, `ITexture`, `ISprite`
   - Implémentation SFML

2. **RenderSystem**
   - Lit Sprite + Position
   - Dessine via IRenderer

3. **Window Management**

4. **Camera basique**

**Validation** : Affichage du starfield + vaisseau

---

### 🟠 Phase 4 : Network Abstraction (Semaine 3-4)

1. **Packet System**
   - Sérialisation binaire
   - Read/Write primitives

2. **Socket Abstraction**
   - UDP socket wrapper (Asio ou raw)
   - Send/Receive

3. **Connection Manager**
   - Liste de connexions actives
   - Heartbeat/timeout

**Validation** : Client peut envoyer/recevoir packets du serveur

---

### 🔴 Phase 5 : Game Logic Systems (Semaine 4)

1. **CollisionSystem**
   - AABB collision detection
   - Event publishing

2. **HealthSystem**
   - Gestion des dégâts/mort

3. **WeaponSystem**
   - Spawn bullets
   - Fire rate

4. **AISystem** (basique)

**Validation** : Prototype jouable

---

### 🟣 Phase 6 : Advanced Features (Semaine 5-7)

1. **Network Sync**
   - Snapshot interpolation
   - Client prediction
   - Server reconciliation

2. **Animation System**

3. **Particle System**

4. **Audio** (optionnel)

5. **Optimisations**
   - Spatial partitioning
   - Object pooling
   - Multithreading (si temps)

---

## 🔗 Intégration avec Client/Server

### Architecture générale
```
┌─────────────────────────────────────────┐
│           R-Type Game Logic             │
│         (game/ folder)                  │
│  - Specific components (Player, Enemy)  │
│  - Specific systems (R-Type rules)      │
└─────────────────────────────────────────┘
         ▲                        ▲
         │                        │
         │   Uses Engine API      │
         │                        │
┌────────┴─────────┐    ┌────────┴─────────┐
│   r-type_client  │    │  r-type_server   │
│                  │    │                   │
│ - Window         │    │ - Authoritative   │
│ - Input          │    │ - Multithreaded   │
│ - Rendering      │    │ - No rendering    │
│ - Prediction     │    │ - Broadcasting    │
└──────────────────┘    └───────────────────┘
         │                        │
         │   Links against        │
         └────────┬───────────────┘
                  │
         ┌────────▼─────────┐
         │   Game Engine    │
         │   (engine/)      │
         │                  │
         │ - ECS            │
         │ - Network        │
         │ - Rendering      │
         │ - Physics        │
         └──────────────────┘
```

### Client utilise :
- ✅ ECS complet
- ✅ Rendering subsystem
- ✅ Input subsystem
- ✅ Network (client-side)
- ✅ Prediction/interpolation

### Server utilise :
- ✅ ECS complet
- ❌ PAS de rendering
- ✅ Network (server-side)
- ✅ Physics/collision (authoritative)
- ✅ Game logic

### Game logic (game/) utilise :
- ✅ Components spécifiques R-Type
- ✅ Systems spécifiques R-Type
- ✅ Assets loading

---

## 📚 Ressources recommandées

### ECS
- [EnTT library](https://github.com/skypjack/entt) (référence pour design)
- [Overwatch ECS Architecture](https://www.youtube.com/watch?v=W3aieHjyNvw)
- [Data-Oriented Design](https://www.dataorienteddesign.com/dodbook/)

### Networking
- [Gaffer on Games - Networking](https://gafferongames.com/categories/networked-physics/)
- [Source Engine Networking](https://developer.valvesoftware.com/wiki/Source_Multiplayer_Networking)
- [Fast-Paced Multiplayer](https://www.gabrielgambetta.com/client-server-game-architecture.html)

### Game Engine Architecture
- "Game Engine Architecture" by Jason Gregory
- [Handmade Hero](https://handmadehero.org/) (educational)

---

## ✅ Checklist finale

### Must-Have pour prototype (Semaine 4)
- [ ] ECS fonctionnel (Entity, Component, System, Registry)
- [ ] MovementSystem
- [ ] RenderSystem (starfield + sprites)
- [ ] CollisionSystem (basique)
- [ ] Network abstraction (send/receive packets)
- [ ] Input handling
- [ ] Resource manager (textures)

### Should-Have pour version finale (Semaine 7)
- [ ] Animation system
- [ ] Particle system
- [ ] Network interpolation
- [ ] Client prediction
- [ ] Spatial partitioning
- [ ] Event system
- [ ] Audio (optionnel)
- [ ] Configuration files

### Nice-to-Have
- [ ] Scripting (Lua)
- [ ] Serialization (save/load)
- [ ] Profiling tools
- [ ] Debug rendering
- [ ] Editor (in-engine)

---

## 🎯 Points critiques

### ⚠️ Pièges à éviter
1. **Over-engineering** : Ne pas créer 50 abstractions si inutile
2. **Couplage** : ECS doit être découplé du rendering/network
3. **Performance prématurée** : Faites marcher avant d'optimiser
4. **Scope creep** : R-Type first, engine générique later

### ✅ Bonnes pratiques
1. **Testez chaque subsystem isolément**
2. **Documentation au fur et à mesure**
3. **Git : commits atomiques par feature**
4. **Code review entre membres**
5. **Profiling régulier** (ne devinez pas les bottlenecks)

---

## 🏁 Conclusion

Le **Game Engine** est la partie la plus complexe mais la plus gratifiante. Suivez ce guide étape par étape, et vous aurez une architecture solide et réutilisable.

**Ordre de priorité :**
1. ECS fonctionnel → Base de tout
2. Rendering → Feedback visuel immédiat
3. Network → Multiplayer
4. Game logic systems → Gameplay
5. Polish → Animations, particles, etc.

Bon courage ! 🚀
