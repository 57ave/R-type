# 🎓 Concepts Clés - Game Engine ECS

> Comprendre les concepts fondamentaux avant de coder

---

## 📚 Table des Matières
1. [Qu'est-ce qu'un ECS ?](#quest-ce-quun-ecs-)
2. [Comment tout fonctionne ensemble](#comment-tout-fonctionne-ensemble)
3. [Entity](#entity)
4. [Component](#component)
5. [System](#system)
6. [Registry](#registry)
7. [Pourquoi ECS > OOP classique ?](#pourquoi-ecs--oop-classique-)
8. [Flux de données complet](#flux-de-données-complet)
9. [Patterns de conception](#patterns-de-conception)
10. [Networking pour jeux multijoueurs](#networking-pour-jeux-multijoueurs)
11. [Performance & Optimisation](#performance--optimisation)
12. [Anti-patterns à éviter](#anti-patterns-à-éviter)

---

## 🧩 Qu'est-ce qu'un ECS ?

**ECS** = **Entity Component System**

Un pattern architectural qui sépare :
- **Données** (Components)
- **Logique** (Systems)
- **Identités** (Entities)

### Philosophie : Composition > Héritage

❌ **Approche classique OOP**
```cpp
class GameObject {
    Position pos;
    Velocity vel;
    virtual void update() = 0;
};

class Player : public GameObject {
    Health health;
    Weapon weapon;
    void update() override { /* ... */ }
};

class Enemy : public GameObject {
    AI ai;
    void update() override { /* ... */ }
};
```

**Problèmes** :
- Hiérarchie rigide
- Diamond problem
- Code dupliqué
- Difficile à étendre

---

✅ **Approche ECS**
```cpp
// Entity = simple ID
Entity player = registry.createEntity();

// Components = pure data
registry.addComponent<Position>(player, {100, 200});
registry.addComponent<Health>(player, {100, 100});
registry.addComponent<Weapon>(player, {10, 1.0f});

// Systems = pure logic
MovementSystem movementSystem;
HealthSystem healthSystem;
```

**Avantages** :
- ✅ Composition flexible
- ✅ Pas d'héritage complexe
- ✅ Cache-friendly
- ✅ Facile à étendre
- ✅ Multithreading friendly

---

## 🔄 Comment tout fonctionne ensemble

### Vue d'ensemble du flux

Le game engine fonctionne selon un cycle simple mais puissant :

```
┌─────────────────────────────────────────────────────────────┐
│                      INITIALIZATION                          │
├─────────────────────────────────────────────────────────────┤
│  1. Create Registry                                          │
│  2. Register Systems (Movement, Collision, Render, etc.)     │
│  3. Load Resources (Textures, Sounds via ResourceManager)    │
│  4. Setup Network (if multiplayer)                           │
│  5. Create initial Entities (player, enemies, etc.)          │
└─────────────────────────────────────────────────────────────┘
                            ↓
┌─────────────────────────────────────────────────────────────┐
│                       GAME LOOP                              │
├─────────────────────────────────────────────────────────────┤
│  EVERY FRAME (60 FPS):                                       │
│                                                              │
│  ┌────────────────────────────────────────────────────┐     │
│  │ 1. INPUT (Client only)                             │     │
│  │    - InputManager reads keyboard/mouse             │     │
│  │    - Updates PlayerInput components                │     │
│  └────────────────────────────────────────────────────┘     │
│                            ↓                                 │
│  ┌────────────────────────────────────────────────────┐     │
│  │ 2. NETWORK RECEIVE                                 │     │
│  │    Client: Receive world state from server         │     │
│  │    Server: Receive input from all clients          │     │
│  └────────────────────────────────────────────────────┘     │
│                            ↓                                 │
│  ┌────────────────────────────────────────────────────┐     │
│  │ 3. SYSTEMS UPDATE (via Registry.update(dt))        │     │
│  │    a. InputSystem    → Apply player inputs         │     │
│  │    b. AISystem       → Update enemy AI             │     │
│  │    c. MovementSystem → Position += Velocity * dt   │     │
│  │    d. PhysicsSystem  → Apply forces                │     │
│  │    e. CollisionSystem→ Detect & resolve collisions │     │
│  │    f. HealthSystem   → Check deaths                │     │
│  │    g. WeaponSystem   → Handle shooting             │     │
│  │    h. AnimationSystem→ Update sprite frames        │     │
│  │    i. LifetimeSystem → Remove expired entities     │     │
│  └────────────────────────────────────────────────────┘     │
│                            ↓                                 │
│  ┌────────────────────────────────────────────────────┐     │
│  │ 4. EVENTS                                          │     │
│  │    - EventBus dispatches collision events          │     │
│  │    - Systems react to events                       │     │
│  └────────────────────────────────────────────────────┘     │
│                            ↓                                 │
│  ┌────────────────────────────────────────────────────┐     │
│  │ 5. NETWORK SEND                                    │     │
│  │    Client: Send input to server                    │     │
│  │    Server: Broadcast world state to clients        │     │
│  └────────────────────────────────────────────────────┘     │
│                            ↓                                 │
│  ┌────────────────────────────────────────────────────┐     │
│  │ 6. RENDER (Client only)                            │     │
│  │    - RenderSystem queries Sprite + Position        │     │
│  │    - Draws via IRenderer abstraction               │     │
│  │    - Camera transforms world → screen              │     │
│  └────────────────────────────────────────────────────┘     │
│                            ↓                                 │
│  ┌────────────────────────────────────────────────────┐     │
│  │ 7. TIME UPDATE                                     │     │
│  │    - Calculate deltaTime                           │     │
│  │    - Update totalTime                              │     │
│  └────────────────────────────────────────────────────┘     │
│                            ↑                                 │
│                            └─────── REPEAT ─────────────┘    │
└─────────────────────────────────────────────────────────────┘
```

### Exemple concret : Un joueur tire un missile

Suivons le flux de données complet :

#### 1. **Input** (Frame N)
```cpp
// InputManager détecte Space pressed
InputManager::update() {
    if (keyboard.isPressed(Space)) {
        auto& input = registry.getComponent<PlayerInput>(localPlayer);
        input.fire = true;
    }
}
```

#### 2. **Network Send** (Client)
```cpp
// Client envoie l'input au serveur
Packet packet;
packet.write(PacketType::INPUT);
packet.write(playerInput);
packet.write(sequenceNumber);
socket.send(packet, serverAddress);
```

#### 3. **Network Receive** (Server)
```cpp
// Serveur reçoit l'input
auto packet = socket.receive();
uint32_t playerId = packet.read<uint32_t>();
PlayerInput input = packet.read<PlayerInput>();

// Met à jour le component du joueur
registry.getComponent<PlayerInput>(players[playerId]) = input;
```

#### 4. **WeaponSystem Update** (Server - Authoritative)
```cpp
// WeaponSystem traite le tir
void WeaponSystem::update(Registry& reg, float dt) {
    auto view = reg.view<Position, Weapon, PlayerInput>();
    
    for (auto entity : view) {
        auto& pos = reg.getComponent<Position>(entity);
        auto& weapon = reg.getComponent<Weapon>(entity);
        auto& input = reg.getComponent<PlayerInput>(entity);
        
        if (input.fire && weapon.canFire(dt)) {
            // Créer le missile
            Entity bullet = reg.createEntity();
            reg.addComponent<Position>(bullet, {pos.x + 50, pos.y});
            reg.addComponent<Velocity>(bullet, {300, 0}); // 300px/s
            reg.addComponent<Sprite>(bullet, {"bullet.png"});
            reg.addComponent<Collider>(bullet, {10, 5});
            reg.addComponent<Damage>(bullet, {10});
            reg.addComponent<Lifetime>(bullet, {5.0f}); // 5 sec
            reg.addComponent<NetworkId>(bullet, {nextNetworkId++});
            
            weapon.lastFireTime = currentTime;
            input.fire = false;
        }
    }
}
```

#### 5. **MovementSystem Update** (même frame)
```cpp
// Le missile commence à bouger immédiatement
void MovementSystem::update(Registry& reg, float dt) {
    auto view = reg.view<Position, Velocity>();
    
    for (auto entity : view) {
        auto& pos = reg.getComponent<Position>(entity);
        auto& vel = reg.getComponent<Velocity>(entity);
        
        pos.x += vel.vx * dt;
        pos.y += vel.vy * dt;
    }
}
// Bullet position: x = 100 + 300 * 0.016 = 104.8
```

#### 6. **CollisionSystem Update** (frame suivante)
```cpp
void CollisionSystem::update(Registry& reg, float dt) {
    auto bullets = reg.view<Position, Collider, Damage>();
    auto enemies = reg.view<Position, Collider, Health>();
    
    for (auto bullet : bullets) {
        auto& bulletPos = reg.getComponent<Position>(bullet);
        auto& bulletCol = reg.getComponent<Collider>(bullet);
        
        for (auto enemy : enemies) {
            auto& enemyPos = reg.getComponent<Position>(enemy);
            auto& enemyCol = reg.getComponent<Collider>(enemy);
            
            if (detector.checkAABB(bulletPos, bulletCol, enemyPos, enemyCol)) {
                // Collision détectée !
                auto& damage = reg.getComponent<Damage>(bullet);
                auto& health = reg.getComponent<Health>(enemy);
                
                health.current -= damage.amount;
                
                // Publish event
                eventBus->publish(CollisionEvent{bullet, enemy});
                
                // Détruire le missile
                reg.destroyEntity(bullet);
            }
        }
    }
}
```

#### 7. **HealthSystem Update**
```cpp
void HealthSystem::update(Registry& reg, float dt) {
    auto view = reg.view<Health>();
    
    for (auto entity : view) {
        auto& health = reg.getComponent<Health>(entity);
        
        if (health.current <= 0) {
            // Publier event de mort
            eventBus->publish(EntityDeathEvent{entity});
            
            // Détruire l'ennemi
            reg.destroyEntity(entity);
        }
    }
}
```

#### 8. **Network Broadcast** (Server)
```cpp
// Serveur envoie le nouvel état à tous les clients
Packet statePacket;
statePacket.write(PacketType::WORLD_STATE);

// Pour chaque entity avec NetworkId
auto networked = registry.view<NetworkId, Position, Velocity>();
for (auto entity : networked) {
    auto& netId = registry.getComponent<NetworkId>(entity);
    auto& pos = registry.getComponent<Position>(entity);
    auto& vel = registry.getComponent<Velocity>(entity);
    
    statePacket.write(netId.id);
    statePacket.write(pos);
    statePacket.write(vel);
    // ... autres components
}

// Broadcast à tous
connectionManager.broadcastToAll(statePacket);
```

#### 9. **Client Receive & Interpolation**
```cpp
// Client reçoit le state
auto packet = socket.receive();

while (packet.hasData()) {
    uint32_t netId = packet.read<uint32_t>();
    Position pos = packet.read<Position>();
    Velocity vel = packet.read<Velocity>();
    
    // Trouver l'entity locale correspondante
    Entity localEntity = networkIdMap[netId];
    
    // Interpolation pour smooth movement
    interpolator.addSnapshot(currentTime, pos);
    
    // Appliquer la position interpolée
    Position smoothPos = interpolator.interpolate(renderTime);
    registry.getComponent<Position>(localEntity) = smoothPos;
}
```

#### 10. **RenderSystem Update** (Client)
```cpp
void RenderSystem::update(Registry& reg, float dt) {
    // Query toutes les entities visibles
    auto view = reg.view<Position, Sprite>();
    
    // Trier par layer (background → foreground)
    std::sort(view.begin(), view.end(), [&](Entity a, Entity b) {
        return reg.getComponent<Sprite>(a).layer < 
               reg.getComponent<Sprite>(b).layer;
    });
    
    // Dessiner
    for (auto entity : view) {
        auto& pos = reg.getComponent<Position>(entity);
        auto& sprite = reg.getComponent<Sprite>(entity);
        
        // Transformer world → screen via Camera
        Vector2f screenPos = camera.worldToScreen({pos.x, pos.y});
        
        // Dessiner via abstraction
        renderer->draw(sprite, screenPos);
    }
    
    renderer->present();
}
```

### Résumé du flux pour un tir de missile

```
Frame N:
  Client: Input détecté (Space) → Envoie au serveur
  
Frame N+1 (Server):
  Server: Reçoit input → WeaponSystem crée bullet entity
         → MovementSystem bouge bullet
         → CollisionSystem détecte hit
         → HealthSystem tue ennemi
         → Broadcast state à clients
  
Frame N+2 (Client):
  Client: Reçoit state → Interpolation → RenderSystem affiche
```

### Rôle de chaque composant

| Composant | Responsabilité | Exemple |
|-----------|---------------|---------|
| **Entity** | Identifiant unique | `Entity bullet = 42` |
| **Components** | Stocke les données | `Position{100, 200}` |
| **Registry** | Orchestre tout | `registry.update(dt)` |
| **Systems** | Logique métier | `MovementSystem::update()` |
| **EventBus** | Communication décuplée | `publish(CollisionEvent)` |
| **ResourceManager** | Cache ressources | `load("bullet.png")` |
| **Network** | Synchronisation | `broadcast(worldState)` |
| **Renderer** | Affichage (abstrait) | `draw(sprite, pos)` |

### Abstraction : Comment client/server injectent leurs implémentations

```cpp
// ENGINE fournit l'interface
class IRenderer {
public:
    virtual void draw(Sprite, Transform) = 0;
};

// CLIENT injecte l'implémentation SFML
class SFMLRenderer : public IRenderer {
    sf::RenderWindow window;
public:
    void draw(Sprite s, Transform t) override {
        // Utilise SFML ici
        window.draw(s.sfmlSprite, t.sfmlTransform);
    }
};

// Dans le code client
int main() {
    Registry registry;
    
    // Client crée et injecte le renderer
    SFMLRenderer renderer;
    RenderSystem renderSystem(&renderer);
    
    registry.addSystem<RenderSystem>(renderSystem);
    // ...
}

// SERVER n'utilise PAS de renderer
int main() {
    Registry registry;
    
    // Pas de RenderSystem !
    registry.addSystem<MovementSystem>();
    registry.addSystem<CollisionSystem>();
    // ...
}
```

---

## 🏷️ Entity

### Définition
Une **Entity** est juste un **ID unique**. Rien d'autre.

```cpp
using Entity = uint32_t;

constexpr Entity NULL_ENTITY = 0;
```

### Pourquoi un simple ID ?
- Léger (4 bytes)
- Rapide à copier
- Facile à sérialiser (network)
- Pas de logique, juste une étiquette

### Génération avec versioning
Pour détecter les "stale entities" (entities détruites puis ID recyclé) :

```cpp
struct Entity {
    uint32_t id;
    uint32_t generation;
};
```

**Exemple** :
```
Entity e1 = {id: 42, gen: 0};
destroyEntity(e1);
Entity e2 = createEntity();  // {id: 42, gen: 1} (même ID, nouvelle génération)

// Tentative d'accès à e1 → Erreur détectée (gen différente)
```

### EntityManager

Responsabilités :
- Générer des IDs uniques
- Recycler les IDs des entities détruites
- Valider si une entity existe encore

```cpp
class EntityManager {
private:
    uint32_t nextId = 1;
    std::queue<uint32_t> freeIds;
    std::set<Entity> alive;
    std::array<uint32_t, MAX_ENTITIES> generations;

public:
    Entity create() {
        uint32_t id;
        if (!freeIds.empty()) {
            id = freeIds.front();
            freeIds.pop();
            generations[id]++;
        } else {
            id = nextId++;
        }
        Entity e = {id, generations[id]};
        alive.insert(e);
        return e;
    }
    
    void destroy(Entity e) {
        alive.erase(e);
        freeIds.push(e.id);
    }
    
    bool isAlive(Entity e) const {
        return alive.count(e) > 0 && 
               generations[e.id] == e.generation;
    }
};
```

---

## 📦 Component

### Définition
Un **Component** est une structure de données **pure** (POD - Plain Old Data).

**Règles** :
- ✅ Pas de méthodes (sauf constructeurs/getters simples)
- ✅ Données publiques
- ✅ Serializable
- ✅ Copyable

### Exemples

```cpp
// ✅ BON : Pure data
struct Position {
    float x;
    float y;
};

struct Velocity {
    float vx;
    float vy;
};

struct Health {
    int current;
    int max;
};

struct Sprite {
    std::string texturePath;
    sf::IntRect textureRect;
    int layer;
};

// ❌ MAUVAIS : Logique dans component
struct BadComponent {
    float x, y;
    
    void update(float dt) {  // ❌ Non !
        x += dt;
    }
};
```

### Component Storage : SparseSet

**Problème** : Comment stocker efficacement des components pour des milliers d'entities ?

**Solution** : **SparseSet**

```cpp
template<typename T>
class SparseSet {
private:
    std::vector<size_t> sparse;     // Entity ID → index dans dense
    std::vector<Entity> dense;      // Liste compacte d'entities
    std::vector<T> components;      // Components (alignés avec dense)

public:
    void insert(Entity e, T component) {
        if (e.id >= sparse.size())
            sparse.resize(e.id + 1, NULL_INDEX);
        
        sparse[e.id] = dense.size();
        dense.push_back(e);
        components.push_back(component);
    }
    
    T& get(Entity e) {
        return components[sparse[e.id]];
    }
    
    void remove(Entity e) {
        size_t indexToRemove = sparse[e.id];
        size_t lastIndex = dense.size() - 1;
        
        // Swap with last
        std::swap(dense[indexToRemove], dense[lastIndex]);
        std::swap(components[indexToRemove], components[lastIndex]);
        sparse[dense[indexToRemove].id] = indexToRemove;
        
        // Remove last
        dense.pop_back();
        components.pop_back();
        sparse[e.id] = NULL_INDEX;
    }
    
    // Itération rapide (contiguous memory)
    auto begin() { return components.begin(); }
    auto end() { return components.end(); }
};
```

**Avantages** :
- ✅ O(1) add/remove/get
- ✅ Itération ultra-rapide (contiguous array)
- ✅ Cache-friendly
- ✅ Pas de trous (compact)

**Visualisation** :
```
Entities: [e5, e2, e8, e1]
sparse:   [_, 3, 1, _, _, 0, _, _, 2]
           ^  ^  ^        ^        ^
          e0 e1 e2       e5       e8
          
dense:      [e5, e2, e8, e1]
components: [C5, C2, C8, C1]  ← Contiguous !
```

---

## ⚙️ System

### Définition
Un **System** est une fonction/classe qui opère sur des **Components**.

**Règles** :
- ✅ Pure logic (pas de state, ou minimal)
- ✅ Operate sur des queries (view)
- ✅ Appelé chaque frame (ou selon besoin)

### Interface

```cpp
class ISystem {
public:
    virtual ~ISystem() = default;
    virtual void update(Registry& registry, float deltaTime) = 0;
    virtual std::string getName() const = 0;
};
```

### Exemple : MovementSystem

```cpp
class MovementSystem : public ISystem {
public:
    void update(Registry& registry, float dt) override {
        // Query toutes les entities avec Position ET Velocity
        auto view = registry.view<Position, Velocity>();
        
        for (auto entity : view) {
            auto& pos = registry.getComponent<Position>(entity);
            auto& vel = registry.getComponent<Velocity>(entity);
            
            // Logique
            pos.x += vel.vx * dt;
            pos.y += vel.vy * dt;
        }
    }
    
    std::string getName() const override {
        return "MovementSystem";
    }
};
```

### Exemple : CollisionSystem

```cpp
class CollisionSystem : public ISystem {
private:
    EventBus* eventBus;
    
public:
    CollisionSystem(EventBus* bus) : eventBus(bus) {}
    
    void update(Registry& registry, float dt) override {
        auto view = registry.view<Position, Collider>();
        
        // Nested loop pour collision detection (optimisable avec QuadTree)
        for (auto entityA : view) {
            auto& posA = registry.getComponent<Position>(entityA);
            auto& colA = registry.getComponent<Collider>(entityA);
            
            for (auto entityB : view) {
                if (entityA == entityB) continue;
                
                auto& posB = registry.getComponent<Position>(entityB);
                auto& colB = registry.getComponent<Collider>(entityB);
                
                if (checkAABB(posA, colA, posB, colB)) {
                    // Publish collision event
                    eventBus->publish(CollisionEvent{entityA, entityB});
                }
            }
        }
    }
};
```

### System Ordering

L'ordre d'exécution **compte** !

```cpp
// Bon ordre
1. InputSystem          // Lit input utilisateur
2. AISystem             // IA décide actions
3. MovementSystem       // Applique velocity → position
4. PhysicsSystem        // Physique (gravité, etc.)
5. CollisionSystem      // Détecte collisions
6. HealthSystem         // Applique dégâts
7. NetworkSyncSystem    // Envoie état au serveur
8. AnimationSystem      // Update animations
9. RenderSystem         // Dessine à l'écran
```

**SystemManager** gère cet ordre :

```cpp
class SystemManager {
private:
    std::vector<std::unique_ptr<ISystem>> systems;
    
public:
    template<typename T, typename... Args>
    T& addSystem(Args&&... args) {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = system.get();
        systems.push_back(std::move(system));
        return *ptr;
    }
    
    void update(Registry& registry, float dt) {
        for (auto& system : systems) {
            system->update(registry, dt);
        }
    }
};
```

---

## 🗃️ Registry

### Définition
Le **Registry** est le **hub central** qui coordonne tout.

### Responsabilités
1. Créer/détruire entities
2. Ajouter/retirer/accéder components
3. Gérer systems
4. Fournir queries (views)

### Implémentation

```cpp
class Registry {
private:
    EntityManager entityManager;
    
    // Map : TypeId → ComponentManager
    std::unordered_map<std::type_index, 
                       std::unique_ptr<IComponentManagerBase>> componentManagers;
    
    SystemManager systemManager;

public:
    // === Entity Management ===
    Entity createEntity() {
        return entityManager.create();
    }
    
    void destroyEntity(Entity e) {
        // Remove from all component pools
        for (auto& [type, manager] : componentManagers) {
            manager->remove(e);
        }
        entityManager.destroy(e);
    }
    
    bool isValid(Entity e) const {
        return entityManager.isAlive(e);
    }
    
    // === Component Management ===
    template<typename T>
    T& addComponent(Entity e, T component) {
        auto& manager = getOrCreateComponentManager<T>();
        return manager.insert(e, std::move(component));
    }
    
    template<typename T>
    T& getComponent(Entity e) {
        auto& manager = getComponentManager<T>();
        return manager.get(e);
    }
    
    template<typename T>
    bool hasComponent(Entity e) const {
        if (!componentManagers.count(typeid(T)))
            return false;
        return getComponentManager<T>().has(e);
    }
    
    template<typename T>
    void removeComponent(Entity e) {
        auto& manager = getComponentManager<T>();
        manager.remove(e);
    }
    
    // === Query / View ===
    template<typename... Components>
    View<Components...> view() {
        return View<Components...>(*this);
    }
    
    // === System Management ===
    template<typename T, typename... Args>
    T& addSystem(Args&&... args) {
        return systemManager.addSystem<T>(std::forward<Args>(args)...);
    }
    
    void update(float dt) {
        systemManager.update(*this, dt);
    }

private:
    template<typename T>
    ComponentManager<T>& getOrCreateComponentManager() {
        auto type = std::type_index(typeid(T));
        
        if (!componentManagers.count(type)) {
            componentManagers[type] = 
                std::make_unique<ComponentManager<T>>();
        }
        
        return static_cast<ComponentManager<T>&>(*componentManagers[type]);
    }
};
```

### View Implementation

```cpp
template<typename... Components>
class View {
private:
    Registry& registry;

public:
    View(Registry& reg) : registry(reg) {}
    
    // Iterator
    class Iterator {
        // Implémentation...
    };
    
    Iterator begin() {
        // Trouve premier entity avec tous les components
    }
    
    Iterator end() {
        // ...
    }
    
    // Helper pour itération
    template<typename Func>
    void each(Func func) {
        for (auto entity : *this) {
            func(entity, registry.getComponent<Components>(entity)...);
        }
    }
};

// Usage
auto view = registry.view<Position, Velocity>();
view.each([](Entity e, Position& pos, Velocity& vel) {
    pos.x += vel.vx;
});
```

---

## 💡 Pourquoi ECS > OOP classique ?

### 1. **Performance (Cache-Friendly)**

❌ **OOP classique**
```
[GameObject1: pos, vel, sprite, health] → RAM addr 0x1000
[GameObject2: pos, vel, sprite, health] → RAM addr 0x5000
[GameObject3: pos, vel, sprite, health] → RAM addr 0x9000
```
→ Cache misses constants

✅ **ECS**
```
Positions:  [pos1, pos2, pos3, pos4, ...] → RAM addr 0x1000 (contiguous)
Velocities: [vel1, vel2, vel3, vel4, ...] → RAM addr 0x2000 (contiguous)
```
→ Excellent cache locality

### 2. **Composition flexible**

```cpp
// Créer différents types d'entities facilement
Entity staticEnemy = registry.createEntity();
registry.addComponent<Position>(staticEnemy, {100, 100});
registry.addComponent<Sprite>(staticEnemy, {"enemy.png"});
registry.addComponent<Health>(staticEnemy, {50, 50});
// Pas de Velocity → static

Entity movingEnemy = registry.createEntity();
registry.addComponent<Position>(movingEnemy, {200, 200});
registry.addComponent<Velocity>(movingEnemy, {10, 0});
registry.addComponent<Sprite>(movingEnemy, {"enemy.png"});
registry.addComponent<Health>(movingEnemy, {50, 50});
// Avec Velocity → bouge
```

### 3. **Multithreading friendly**

Systems peuvent tourner en parallèle si pas de dépendances :

```cpp
// Paralléliser systems indépendants
std::thread t1([&]() { movementSystem.update(registry, dt); });
std::thread t2([&]() { animationSystem.update(registry, dt); });

t1.join();
t2.join();

// Puis systems dépendants
collisionSystem.update(registry, dt);
```

---

## 🔀 Flux de données complet

### Architecture en couches

```
┌─────────────────────────────────────────────────────────┐
│                    APPLICATION LAYER                     │
│              (Client r-type_client.cpp)                  │
│              (Server r-type_server.cpp)                  │
└─────────────────────────────────────────────────────────┘
                          ↓ uses
┌─────────────────────────────────────────────────────────┐
│                   GAME LOGIC LAYER                       │
│                    (game/ folder)                        │
│  - Game-specific components (Player, Enemy, Bullet)      │
│  - Game-specific systems (PlayerControl, EnemyAI)        │
└─────────────────────────────────────────────────────────┘
                          ↓ uses
┌─────────────────────────────────────────────────────────┐
│                  GAME ENGINE LAYER                       │
│                   (engine/ folder)                       │
│                                                          │
│  ┌──────────┐  ┌────────┐  ┌─────────┐  ┌──────────┐  │
│  │   ECS    │  │  Core  │  │ Network │  │  Render  │  │
│  │          │  │        │  │         │  │ (abstract)│  │
│  │ Registry │  │  Time  │  │ Packet  │  │ IRenderer │  │
│  │ Entity   │  │ Logger │  │ Socket  │  │ ITexture  │  │
│  │Component │  │ Events │  │ ConnMgr │  │  Camera   │  │
│  │ System   │  │Resource│  │         │  │           │  │
│  └──────────┘  └────────┘  └─────────┘  └──────────┘  │
│                                                          │
│  ┌──────────┐  ┌────────────────────────────────────┐  │
│  │ Physics  │  │      Common Systems                │  │
│  │          │  │  Movement, Collision, Animation    │  │
│  │Collision │  │  Lifetime, Particle                │  │
│  │QuadTree  │  │                                    │  │
│  └──────────┘  └────────────────────────────────────┘  │
└─────────────────────────────────────────────────────────┘
```

### Cycle de vie d'une Entity

```
CREATION
   ↓
1. registry.createEntity()
   → EntityManager génère ID unique
   → Ajoute à la liste des entities vivantes
   ↓
2. registry.addComponent<Position>(entity, {x, y})
   → ComponentManager<Position> stocke dans SparseSet
   → Entity est maintenant "Position-aware"
   ↓
3. registry.addComponent<Velocity>(entity, {vx, vy})
   → ComponentManager<Velocity> stocke
   → Entity a maintenant Position + Velocity
   ↓
4. registry.addComponent<Sprite>(entity, {"texture.png"})
   → Entity devient visible
   
GAME LOOP
   ↓
5. MovementSystem.update()
   → Query view<Position, Velocity>
   → Entity apparaît dans les résultats
   → pos.x += vel.vx * dt
   ↓
6. RenderSystem.update()
   → Query view<Position, Sprite>
   → Entity apparaît dans les résultats
   → renderer->draw(sprite, position)
   ↓
7. CollisionSystem.update()
   → Détecte collision
   → EventBus.publish(CollisionEvent{entity, other})
   ↓
8. HealthSystem reçoit event
   → health.current -= damage
   → if (health.current <= 0) → destroyEntity
   
DESTRUCTION
   ↓
9. registry.destroyEntity(entity)
   → Retire de tous les ComponentManagers
   → EntityManager recycle l'ID
   → Entity n'existe plus
```

### Interactions entre subsystems

#### 1. ECS + Core

```cpp
// Time fournit deltaTime aux Systems
void Registry::update(float dt) {
    // dt vient de Time::getDeltaTime()
    systemManager.update(*this, dt);
}

// Logger utilisé partout
Logger::info("Entity created: {}", entity.id);

// EventBus pour communication
eventBus.publish(CollisionEvent{e1, e2});
```

#### 2. ECS + Network

```cpp
// Synchronisation des entities
void NetworkSyncSystem::update(Registry& reg, float dt) {
    auto networked = reg.view<NetworkId, Position, Velocity>();
    
    Packet packet;
    for (auto entity : networked) {
        auto& netId = reg.getComponent<NetworkId>(entity);
        auto& pos = reg.getComponent<Position>(entity);
        
        packet.write(netId.id);
        packet.write(pos.x);
        packet.write(pos.y);
    }
    
    socket->send(packet, serverAddress);
}
```

#### 3. ECS + Rendering

```cpp
// RenderSystem lit components et dessine
void RenderSystem::update(Registry& reg, float dt) {
    auto view = reg.view<Position, Sprite>();
    
    for (auto entity : view) {
        auto& pos = reg.getComponent<Position>(entity);
        auto& sprite = reg.getComponent<Sprite>(entity);
        
        // IRenderer = abstraction injectée par le client
        renderer->draw(sprite, pos);
    }
}

// ResourceManager charge les textures
auto texture = resourceMgr.load<Texture>("player.png");
sprite.texture = texture.get();
```

#### 4. ECS + Physics

```cpp
// CollisionSystem utilise Physics
void CollisionSystem::update(Registry& reg, float dt) {
    auto view = reg.view<Position, Collider>();
    
    // Optimisation avec QuadTree
    quadTree.clear();
    for (auto entity : view) {
        auto& pos = reg.getComponent<Position>(entity);
        auto& col = reg.getComponent<Collider>(entity);
        quadTree.insert(entity, {pos.x, pos.y, col.width, col.height});
    }
    
    // Détection
    for (auto entity : view) {
        auto nearby = quadTree.query(getBounds(entity));
        
        for (auto other : nearby) {
            if (collisionDetector.checkAABB(...)) {
                eventBus->publish(CollisionEvent{entity, other});
            }
        }
    }
}
```

### Communication inter-systems via EventBus

```cpp
// System A publie un event
class CollisionSystem : public ISystem {
    void update(Registry& reg, float dt) override {
        // Détecte collision
        eventBus->publish(CollisionEvent{bullet, enemy});
    }
};

// System B réagit à l'event
class ScoreSystem : public ISystem {
    ScoreSystem(EventBus* bus) {
        bus->subscribe<CollisionEvent>([this](const CollisionEvent& e) {
            // Check si bullet + enemy
            if (isBullet(e.entityA) && isEnemy(e.entityB)) {
                score += 100;
            }
        });
    }
};

// System C réagit aussi
class ParticleSystem : public ISystem {
    ParticleSystem(EventBus* bus) {
        bus->subscribe<CollisionEvent>([this](const CollisionEvent& e) {
            // Spawn explosion particles
            spawnExplosion(e.position);
        });
    }
};
```

### Dépendances entre subsystems

```
                    ┌──────────┐
                    │ Registry │ ← Hub central
                    └──────────┘
                         │
        ┌────────────────┼────────────────┐
        ↓                ↓                ↓
   ┌─────────┐    ┌──────────┐    ┌──────────┐
   │ Systems │    │Components│    │ Entities │
   └─────────┘    └──────────┘    └──────────┘
        │
        ↓ uses
   ┌─────────────────────────────────────────┐
   │  Time, Logger, EventBus, ResourceMgr    │
   │  CollisionDetector, IRenderer, Network  │
   └─────────────────────────────────────────┘
```

**Règles importantes** :
- ✅ Systems peuvent utiliser Core (Time, Logger, EventBus)
- ✅ Systems peuvent utiliser Physics, Network
- ✅ Systems communiquent via EventBus (découplés)
- ❌ Systems NE doivent PAS communiquer directement
- ❌ Components NE doivent PAS contenir de logique
- ❌ Components NE connaissent PAS les Systems

### Exemple complet : Du client au serveur

```cpp
// ===== CLIENT SIDE =====
void ClientGameLoop::run() {
    Registry registry;
    
    // Setup
    SFMLRenderer renderer;  // Client injecte SFML
    RenderSystem renderSys(&renderer);
    NetworkClient network("127.0.0.1", 8080);
    
    registry.addSystem<MovementSystem>();
    registry.addSystem<RenderSystem>(renderSys);
    registry.addSystem<NetworkSyncSystem>(&network);
    
    // Create local player
    Entity player = registry.createEntity();
    registry.addComponent<Position>(player, {100, 100});
    registry.addComponent<Velocity>(player, {0, 0});
    registry.addComponent<Sprite>(player, {"player.png"});
    registry.addComponent<PlayerInput>(player, {});
    
    // Game loop
    while (running) {
        float dt = time.getDeltaTime();
        
        // 1. Input
        inputManager.update();
        
        // 2. Receive from server
        network.receiveWorldState(registry);
        
        // 3. Update systems
        registry.update(dt);  // Appelle tous les systems
        
        // 4. Send to server
        network.sendInput(registry);
        
        // 5. Render (happens in RenderSystem)
    }
}

// ===== SERVER SIDE =====
void ServerGameLoop::run() {
    Registry registry;
    
    // Setup (NO RENDERING!)
    NetworkServer network(8080);
    
    registry.addSystem<MovementSystem>();
    registry.addSystem<CollisionSystem>();
    registry.addSystem<HealthSystem>();
    registry.addSystem<EnemyAISystem>();
    // Pas de RenderSystem !
    
    // Game loop
    while (running) {
        float dt = time.getDeltaTime();
        
        // 1. Receive inputs from all clients
        network.receiveAllInputs(registry);
        
        // 2. Update authoritative state
        registry.update(dt);
        
        // 3. Broadcast state to all clients
        network.broadcastWorldState(registry);
    }
}
```

Ce flux montre comment **tous les subsystems collaborent** pour créer un jeu multijoueur fonctionnel, avec une **séparation claire** entre engine (réutilisable) et game logic (spécifique à R-Type).

---

## 🎨 Patterns de conception

### 1. **Event Bus (Pub/Sub)**

Pour communication décuplée entre systems.

```cpp
class EventBus {
private:
    std::unordered_map<std::type_index, 
                       std::vector<std::function<void(void*)>>> subscribers;

public:
    template<typename T>
    void subscribe(std::function<void(const T&)> callback) {
        auto wrapper = [callback](void* data) {
            callback(*static_cast<T*>(data));
        };
        subscribers[typeid(T)].push_back(wrapper);
    }
    
    template<typename T>
    void publish(const T& event) {
        auto it = subscribers.find(typeid(T));
        if (it != subscribers.end()) {
            for (auto& callback : it->second) {
                callback(const_cast<T*>(&event));
            }
        }
    }
};

// Usage
struct CollisionEvent {
    Entity a, b;
};

eventBus.subscribe<CollisionEvent>([](const CollisionEvent& e) {
    std::cout << "Collision between " << e.a << " and " << e.b << std::endl;
});

eventBus.publish(CollisionEvent{player, enemy});
```

### 2. **Resource Manager (Flyweight)**

Cache de ressources (textures, sons, etc.)

```cpp
template<typename T>
class ResourceManager {
private:
    std::unordered_map<std::string, std::shared_ptr<T>> resources;
    
public:
    std::shared_ptr<T> load(const std::string& path) {
        // Check cache
        if (resources.count(path)) {
            return resources[path];
        }
        
        // Load
        auto resource = std::make_shared<T>();
        if (!resource->loadFromFile(path)) {
            throw std::runtime_error("Failed to load: " + path);
        }
        
        resources[path] = resource;
        return resource;
    }
    
    std::shared_ptr<T> get(const std::string& path) {
        if (!resources.count(path)) {
            return load(path);
        }
        return resources[path];
    }
};
```

### 3. **Object Pool**

Réutiliser entities/objects pour éviter allocations.

```cpp
class BulletPool {
private:
    std::vector<Entity> available;
    Registry& registry;

public:
    Entity acquire() {
        if (available.empty()) {
            // Create new
            Entity e = registry.createEntity();
            registry.addComponent<Position>(e, {0, 0});
            registry.addComponent<Velocity>(e, {0, 0});
            registry.addComponent<Sprite>(e, {"bullet.png"});
            return e;
        }
        
        Entity e = available.back();
        available.pop_back();
        return e;
    }
    
    void release(Entity e) {
        available.push_back(e);
    }
};
```

---

## 🌐 Networking pour jeux multijoueurs

### Architecture Client-Server

```
Client1 ←--→ Server ←--→ Client2
   ↓           ↓           ↓
Prediction  Authority  Interpolation
```

### Principes clés

#### 1. **Server Authoritative**
Le serveur a toujours raison.

```cpp
// Server
void ServerLogic::processPlayerInput(uint32_t playerId, Input input) {
    Entity player = playerEntities[playerId];
    
    auto& vel = registry.getComponent<Velocity>(player);
    
    // Apply input (server validates)
    if (input.moveLeft) vel.vx = -100;
    if (input.moveRight) vel.vx = 100;
    // ...
    
    // Server updates position (authoritative)
    registry.update(deltaTime);
}
```

#### 2. **Client-Side Prediction**
Client prédit mouvement localement pour éviter lag visible.

```cpp
// Client
void ClientPredictor::applyInput(Input input) {
    // Store input avec sequence number
    pendingInputs.push_back({sequenceNumber++, input});
    
    // Apply locally (prediction)
    applyInputToEntity(localPlayer, input);
    
    // Send to server
    sendInputPacket(input, sequenceNumber);
}
```

#### 3. **Server Reconciliation**
Client corrige si prédiction était fausse.

```cpp
void ClientPredictor::onServerUpdate(ServerState state) {
    // Remove inputs que le serveur a déjà traités
    pendingInputs.erase(
        std::remove_if(pendingInputs.begin(), pendingInputs.end(),
            [&](auto& input) { return input.seq <= state.lastProcessedSeq; }),
        pendingInputs.end()
    );
    
    // Set to server state
    setPlayerPosition(localPlayer, state.position);
    
    // Re-apply pending inputs
    for (auto& input : pendingInputs) {
        applyInputToEntity(localPlayer, input.data);
    }
}
```

#### 4. **Entity Interpolation**
Smooth movement des autres joueurs.

```cpp
class NetworkInterpolator {
private:
    std::deque<Snapshot> snapshots;
    
public:
    void addSnapshot(float timestamp, Position pos) {
        snapshots.push_back({timestamp, pos});
        
        // Keep only recent
        while (snapshots.size() > 10) {
            snapshots.pop_front();
        }
    }
    
    Position interpolate(float renderTime) {
        // Find 2 snapshots autour de renderTime
        auto it = std::lower_bound(snapshots.begin(), snapshots.end(), renderTime);
        
        if (it == snapshots.begin() || it == snapshots.end())
            return snapshots.back().position;
        
        auto next = it;
        auto prev = std::prev(it);
        
        // Lerp
        float t = (renderTime - prev->time) / (next->time - prev->time);
        return lerp(prev->position, next->position, t);
    }
};
```

---

## ⚡ Performance & Optimisation

### 1. **Spatial Partitioning (QuadTree)**

Au lieu de tester toutes les paires de collisions (O(n²)), utilise QuadTree (O(n log n)).

```cpp
class QuadTree {
private:
    Rect bounds;
    std::vector<Entity> entities;
    std::unique_ptr<QuadTree> children[4];
    const size_t MAX_ENTITIES = 4;

public:
    void insert(Entity e, Position pos) {
        if (!bounds.contains(pos))
            return;
        
        if (entities.size() < MAX_ENTITIES) {
            entities.push_back(e);
        } else {
            subdivide();
            for (auto& child : children) {
                child->insert(e, pos);
            }
        }
    }
    
    std::vector<Entity> query(Rect area) {
        std::vector<Entity> result;
        
        if (!bounds.intersects(area))
            return result;
        
        for (auto e : entities) {
            if (area.contains(getPosition(e))) {
                result.push_back(e);
            }
        }
        
        for (auto& child : children) {
            auto childResult = child->query(area);
            result.insert(result.end(), childResult.begin(), childResult.end());
        }
        
        return result;
    }
};
```

### 2. **Component Packing**

Aligne components pour cache efficiency.

```cpp
// ❌ Mauvais (padding)
struct BadComponent {
    bool flag;     // 1 byte
    // 3 bytes padding
    float value;   // 4 bytes
    // Total: 8 bytes
};

// ✅ Bon
struct GoodComponent {
    float value;   // 4 bytes
    bool flag;     // 1 byte
    // 3 bytes padding à la fin (pas au milieu)
    // Total: 8 bytes mais mieux organisé
};
```

### 3. **Batch Rendering**

Group draw calls par texture/layer.

```cpp
void RenderSystem::update(Registry& reg, float dt) {
    // Group by texture
    std::map<Texture*, std::vector<RenderData>> batches;
    
    auto view = reg.view<Position, Sprite>();
    for (auto e : view) {
        auto& pos = reg.getComponent<Position>(e);
        auto& spr = reg.getComponent<Sprite>(e);
        
        batches[spr.texture].push_back({pos, spr});
    }
    
    // Draw batches (1 draw call per texture)
    for (auto& [texture, data] : batches) {
        renderer->drawBatch(texture, data);
    }
}
```

---

## ⚠️ Anti-patterns à éviter

### 1. ❌ Logique dans Components
```cpp
// ❌ NON !
struct BadComponent {
    float x, y;
    void update(float dt) {  // Logique ici = anti-pattern
        x += dt;
    }
};

// ✅ OUI
struct Position { float x, y; };

class MovementSystem {
    void update(...) {
        // Logique ici
    }
};
```

### 2. ❌ God Entity
```cpp
// ❌ Entity avec 50 components
Entity player = registry.createEntity();
registry.addComponent<Position>(player, ...);
registry.addComponent<Velocity>(player, ...);
registry.addComponent<Sprite>(player, ...);
// ... 47 autres components

// ✅ Séparer en sous-entities si nécessaire
```

### 3. ❌ Systems qui communiquent directement
```cpp
// ❌ Couplage fort
class HealthSystem {
    RenderSystem* renderSys;  // ❌
    
    void onDeath(Entity e) {
        renderSys->showExplosion(e);  // ❌
    }
};

// ✅ Event bus
class HealthSystem {
    EventBus* eventBus;
    
    void onDeath(Entity e) {
        eventBus->publish(EntityDeathEvent{e});  // ✅
    }
};
```

### 4. ❌ Nested loops sans optimisation
```cpp
// ❌ O(n²) sans QuadTree
for (auto a : entities) {
    for (auto b : entities) {
        checkCollision(a, b);
    }
}

// ✅ Avec QuadTree ou spatial hashing
auto candidates = quadTree.query(area);
for (auto a : candidates) {
    // ...
}
```

---

## ✅ Checklist de Compréhension

Après avoir lu ce document, tu devrais pouvoir répondre :

- [ ] Qu'est-ce qu'une Entity ? (ID unique)
- [ ] Qu'est-ce qu'un Component ? (Pure data)
- [ ] Qu'est-ce qu'un System ? (Pure logic)
- [ ] Pourquoi SparseSet ? (O(1) + cache-friendly)
- [ ] Pourquoi ECS > OOP ? (Performance, flexibilité)
- [ ] Qu'est-ce que le Registry ? (Hub central)
- [ ] Comment fonctionne Client-Side Prediction ? (Apply input locally + reconcile)
- [ ] Qu'est-ce qu'un QuadTree ? (Spatial partitioning pour collisions)

Si tu peux répondre, tu es prêt à implémenter ! 🚀

---

**Prochaine étape** : Lire [ROADMAP.md](./ROADMAP.md) et commencer à coder !
