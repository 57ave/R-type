# Game Engine Quick Reference - R-Type

## 📁 Fichiers créés

1. **`ENGINE_IMPLEMENTATION_GUIDE.md`** - Guide complet d'implémentation (ce fichier)
2. **`engine_architecture.puml`** - Diagramme PlantUML de l'architecture complète
3. **`client_server_flow.puml`** - Diagramme de flux client/serveur
4. **`ecs_detailed.puml`** - Diagramme détaillé de l'ECS

## 🚀 Démarrage rapide

### Ordre d'implémentation recommandé

#### Phase 1 : ECS Core (Semaine 1-2)
```cpp
// À implémenter dans engine/ecs/
1. Entity.hpp          - typedef uint32_t Entity;
2. EntityManager.hpp   - Gestion des IDs
3. ComponentManager.hpp - SparseSet<T>
4. Registry.hpp        - Hub central
5. System.hpp          - Interface ISystem
6. SystemManager.hpp   - Orchestration des systems
```

#### Phase 2 : Core Utilities (Semaine 2)
```cpp
// À implémenter dans engine/core/
1. Time.hpp            - Delta time
2. Logger.hpp          - Logging
3. ResourceManager.hpp - Cache de ressources
4. EventBus.hpp        - Events
5. Config.hpp          - Configuration
```

#### Phase 3 : Rendering (Semaine 3)
```cpp
// À implémenter dans engine/rendering/
1. IRenderer.hpp       - Interface abstraction
2. SFMLRenderer.hpp    - Implémentation SFML
3. Window.hpp          - Fenêtre
4. Camera.hpp          - Caméra 2D
5. RenderSystem.hpp    - Système de rendu
```

#### Phase 4 : Network (Semaine 3-4)
```cpp
// À implémenter dans engine/network/
1. Packet.hpp          - Sérialisation
2. ISocket.hpp         - Interface socket
3. UDPSocket.hpp       - Implémentation UDP
4. Connection.hpp      - Connexion
5. ConnectionManager.hpp - Gestion multi-client
```

#### Phase 5 : Physics & Game Logic (Semaine 4)
```cpp
// À implémenter dans engine/physics/
1. CollisionDetector.hpp - AABB, Circle
2. CollisionSystem.hpp   - Système de collision
3. QuadTree.hpp          - Spatial partitioning

// Systems communs
4. MovementSystem.hpp    - Position + Velocity
5. AnimationSystem.hpp   - Animation de sprites
6. LifetimeSystem.hpp    - Destruction temporisée
```

---

## 📝 Checklist de développement

### Fondations ECS
- [ ] `Entity` type défini (uint32_t)
- [ ] `EntityManager` : create/destroy/isAlive
- [ ] `ComponentManager<T>` avec SparseSet
- [ ] `Registry` : addComponent, getComponent, hasComponent
- [ ] `View<Components...>` pour itération
- [ ] `ISystem` interface
- [ ] `SystemManager` : addSystem, update
- [ ] Tests unitaires pour l'ECS

### Core Systems
- [ ] `Time` : deltaTime, totalTime
- [ ] `Logger` : info/warning/error
- [ ] `ResourceManager<Texture>` : load/get/unload
- [ ] `EventBus` : subscribe/publish
- [ ] `Config` : load JSON/TOML
- [ ] `InputManager` : keyboard/mouse

### Rendering (Client uniquement)
- [ ] Interface `IRenderer`
- [ ] `SFMLRenderer` implémentation
- [ ] `Window` management
- [ ] `Camera` 2D
- [ ] `RenderSystem` : draw sprites
- [ ] Support des layers de rendu

### Network
- [ ] `Packet` : write<T>/read<T>
- [ ] Interface `ISocket`
- [ ] `UDPSocket` avec Asio
- [ ] `Connection` : send/receive
- [ ] `ConnectionManager` : broadcast
- [ ] Heartbeat/timeout
- [ ] Protocol documentation

### Physics
- [ ] `CollisionDetector` : AABB
- [ ] `CollisionSystem`
- [ ] `QuadTree` (optionnel)
- [ ] Events de collision

### Game Systems
- [ ] `MovementSystem`
- [ ] `AnimationSystem`
- [ ] `HealthSystem`
- [ ] `WeaponSystem`
- [ ] `AISystem` (basique)

### Advanced (Partie 2)
- [ ] Network interpolation
- [ ] Client prediction
- [ ] Server reconciliation
- [ ] Particle system
- [ ] Audio system (optionnel)

---

## 🧩 Patterns de code

### Créer une entité
```cpp
Registry registry;

Entity player = registry.createEntity();
registry.addComponent<Position>(player, Position{100.f, 200.f});
registry.addComponent<Velocity>(player, Velocity{50.f, 0.f});
registry.addComponent<Sprite>(player, Sprite{"player.png"});
```

### Implémenter un System
```cpp
class MovementSystem : public ISystem {
public:
    void update(Registry& registry, float dt) override {
        auto view = registry.view<Position, Velocity>();
        
        for (auto entity : view) {
            auto& pos = registry.getComponent<Position>(entity);
            auto& vel = registry.getComponent<Velocity>(entity);
            
            pos.x += vel.vx * dt;
            pos.y += vel.vy * dt;
        }
    }
};
```

### Enregistrer et exécuter systems
```cpp
Registry registry;

registry.addSystem<MovementSystem>();
registry.addSystem<CollisionSystem>();
registry.addSystem<RenderSystem>();

// Game loop
while (window.isOpen()) {
    float dt = clock.restart().asSeconds();
    registry.update(dt); // Appelle tous les systems
}
```

### Event publishing/subscribing
```cpp
EventBus eventBus;

// Subscribe
auto handle = eventBus.subscribe<CollisionEvent>([](const CollisionEvent& e) {
    std::cout << "Collision between " << e.entityA << " and " << e.entityB << std::endl;
});

// Publish
eventBus.publish(CollisionEvent{player, enemy});
```

### Network packet
```cpp
// Création
Packet packet;
packet.write<uint8_t>(PacketType::PLAYER_MOVE);
packet.write<uint32_t>(playerId);
packet.write<float>(position.x);
packet.write<float>(position.y);

// Envoi
socket->send(packet, serverAddress);

// Réception
auto receivedPacket = socket->receive();
if (receivedPacket) {
    auto type = receivedPacket->read<uint8_t>();
    auto id = receivedPacket->read<uint32_t>();
    auto x = receivedPacket->read<float>();
    auto y = receivedPacket->read<float>();
}
```

---

## 🔧 CMakeLists.txt structure

```cmake
# engine/CMakeLists.txt
add_library(engine STATIC
    # ECS
    src/ecs/EntityManager.cpp
    src/ecs/Registry.cpp
    src/ecs/SystemManager.cpp
    
    # Core
    src/core/Time.cpp
    src/core/Logger.cpp
    src/core/EventBus.cpp
    
    # Network
    src/network/Packet.cpp
    src/network/UDPSocket.cpp
    src/network/Connection.cpp
    
    # Rendering
    src/rendering/SFMLRenderer.cpp
    src/rendering/Window.cpp
    src/rendering/Camera.cpp
    src/rendering/RenderSystem.cpp
    
    # Physics
    src/physics/CollisionDetector.cpp
    src/physics/CollisionSystem.cpp
    
    # Systems
    src/systems/MovementSystem.cpp
    src/systems/AnimationSystem.cpp
)

target_include_directories(engine PUBLIC include)

# Dependencies
find_package(SFML COMPONENTS graphics window system REQUIRED)
find_package(Boost COMPONENTS asio REQUIRED)

target_link_libraries(engine 
    PUBLIC 
        sfml-graphics 
        sfml-window 
        sfml-system
        Boost::asio
)
```

---

## 📊 Components typiques pour R-Type

```cpp
// Transform components
struct Position { float x, y; };
struct Velocity { float vx, vy; };
struct Rotation { float angle; };

// Rendering
struct Sprite {
    std::string texturePath;
    sf::IntRect textureRect;
    int layer = 0;
};

struct Animator {
    std::vector<sf::IntRect> frames;
    float frameTime;
    float elapsedTime = 0.f;
    size_t currentFrame = 0;
    bool loop = true;
};

// Physics
struct Collider {
    float width, height;
    bool isTrigger = false;
};

struct RigidBody {
    float mass = 1.f;
    bool useGravity = false;
};

// Gameplay
struct Health {
    int current;
    int max;
};

struct Damage {
    int amount;
};

struct Weapon {
    float fireRate;
    float lastFireTime = 0.f;
    int ammo = -1; // -1 = infinite
};

struct PlayerInput {
    bool moveUp, moveDown, moveLeft, moveRight;
    bool fire;
};

struct AIController {
    enum class State { Idle, Patrol, Chase, Attack };
    State state = State::Patrol;
    float detectionRadius = 200.f;
};

// Network
struct NetworkId {
    uint32_t id;
};

struct Replicated {
    // Mark entity as needing network sync
};

// Utility
struct Lifetime {
    float remaining; // Seconds before destruction
};

struct Tag {
    std::string name;
};
```

---

## 🎯 Questions fréquentes

### Q: Pourquoi ECS plutôt qu'héritage ?
**R:** L'ECS offre :
- Flexibilité (composition vs héritage)
- Performance (cache-friendly)
- Découplage (systems indépendants)
- Facilité de maintenance

### Q: Comment partager du code entre client et serveur ?
**R:** Le game engine et la game logic sont partagés. Seul le rendering est client-only.

```
game engine (shared) ← client + server
game logic (shared)  ← client + server
rendering (client)   ← client only
```

### Q: Quelle lib réseau utiliser ?
**R:** Boost.Asio recommandé (abstrait). Sinon raw sockets avec abstraction obligatoire.

### Q: Comment gérer la latence ?
**R:** 
1. Server authoritative (serveur fait loi)
2. Client prediction (responsive feel)
3. Server reconciliation (correction)
4. Interpolation (smooth movement)

### Q: SparseSet vs autres structures ?
**R:** SparseSet = bon compromis entre performance et simplicité. Alternativement : PackedArray, Archetype-based (plus complexe).

---

## 📚 Ressources essentielles

### Documentation
- EnTT (reference ECS): https://github.com/skypjack/entt
- Gaffer on Games (networking): https://gafferongames.com
- Game Programming Patterns: https://gameprogrammingpatterns.com

### Vidéos
- "Overwatch Gameplay Architecture" (GDC)
- "Building a Data-Oriented Entity System" (CppCon)

### Libs à étudier (pour s'inspirer)
- **EnTT** : ECS performant
- **Flecs** : ECS avec queries avancées
- **SFML** : Graphics/Window
- **Boost.Asio** : Networking

---

## ⚠️ Pièges à éviter

1. **Ne pas tester au fur et à mesure** → Tests unitaires dès le début
2. **Over-abstraction** → YAGNI (You Aren't Gonna Need It)
3. **Ignorer la performance** → Profile avant d'optimiser
4. **Coupling tight** → Respecter les abstractions
5. **Pas de documentation** → Doc inline + external
6. **Commit géants** → Commits atomiques
7. **Multithreading prématuré** → Single-threaded d'abord

---

## ✅ Validation des étapes

Après chaque phase, vérifiez :

### Phase 1 (ECS)
```cpp
// Test simple
Registry reg;
Entity e = reg.createEntity();
reg.addComponent<Position>(e, {10, 20});
assert(reg.hasComponent<Position>(e));
auto& pos = reg.getComponent<Position>(e);
assert(pos.x == 10 && pos.y == 20);
```

### Phase 2 (Core)
```cpp
// Test ResourceManager
ResourceManager<Texture> textures;
auto tex = textures.load("player.png");
assert(tex != nullptr);
auto tex2 = textures.get("player.png");
assert(tex == tex2); // Même instance (cache)
```

### Phase 3 (Rendering)
- Affichage d'un starfield scrolling
- Affichage d'un sprite de vaisseau
- Mouvement fluide (60 FPS)

### Phase 4 (Network)
```cpp
// Test packet
Packet p;
p.write<int>(42);
p.write<std::string>("hello");
auto i = p.read<int>();
auto s = p.read<std::string>();
assert(i == 42 && s == "hello");
```

### Phase 5 (Prototype jouable)
- ✅ Client se connecte au serveur
- ✅ Vaisseau se déplace (input)
- ✅ Tir de missiles
- ✅ Collisions détectées
- ✅ Ennemis apparaissent
- ✅ Multi-joueurs fonctionne

---

Bon développement ! 🚀
