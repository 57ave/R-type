# Architecture

## Overview

R-Type uses a client-server authoritative model. The server runs the full game simulation (ECS, physics, AI, collisions). Clients are thin terminals: they capture input, send it to the server, receive world snapshots, and render them.

```
[Client Input] → UDP → [Server Simulation] → UDP → [Client Rendering]
```

The server ticks at **60 Hz**. World snapshots are broadcast at **10 Hz**. Clients interpolate between received states to render smoothly at up to 144 Hz.

---

## Module Dependency

```
         ┌──────────────────────────┐
         │          Game            │
         │  states, AI, factories   │
         └────────────┬─────────────┘
                      │ depends on
         ┌────────────▼─────────────┐
         │          Engine          │
         │  ECS, rendering, audio   │
         │  networking, scripting   │
         └──────┬──────────┬────────┘
                │          │
       ┌────────▼──┐  ┌────▼────────┐
       │  Client   │  │   Server    │
       │ input+render│  │ auth logic │
       └───────────┘  └─────────────┘
```

- **Engine** is game-agnostic. It provides all subsystems.
- **Game** contains all R-Type-specific logic and depends on Engine.
- **Client** and **Server** both depend on Engine and are built as separate executables.

---

## Engine Subsystems

| Subsystem | Location | Role |
|---|---|---|
| ECS | `engine/include/ecs/` | Entity/component/system management |
| Rendering | `engine/include/rendering/` | SFML window, textures, sprites |
| Audio | `engine/include/systems/AudioSystem.hpp` | Sound playback and looping |
| Input | `engine/include/systems/InputSystem.hpp` | Keyboard/gamepad input capture |
| Networking | `engine/include/network/` | UDP client/server, rooms, protocol |
| Scripting | `engine/include/scripting/` | Lua state, bindings, hot-reload |

---

## ECS

The ECS follows a coordinator pattern with three managers behind a single interface.

### EntityManager

Entities are plain integer IDs. The manager tracks which IDs are active and stores a `Signature` (bitset) per entity describing which components it has.

```cpp
Entity e = coordinator.CreateEntity();
coordinator.DestroyEntity(e);
```

### ComponentManager

Components are stored in contiguous `ComponentArray<T>` arrays for cache locality. A type registry maps component types to integer indices.

```cpp
coordinator.RegisterComponent<Position>();
coordinator.AddComponent(e, Position{100.f, 200.f});
Position& p = coordinator.GetComponent<Position>(e);
coordinator.RemoveComponent<Position>(e);
```

All operations are O(1).

### SystemManager

Each system declares a `Signature` (the set of components it requires). The manager maintains a set of matching entities per system and updates it when entity signatures change.

```cpp
auto sys = coordinator.RegisterSystem<MovementSystem>();
Signature sig;
sig.set(coordinator.GetComponentType<Position>());
sig.set(coordinator.GetComponentType<Velocity>());
coordinator.SetSystemSignature<MovementSystem>(sig);
```

### Coordinator

Single entry point that wraps all three managers. Also manages `NetworkId` mappings for entity synchronization across client and server.

---

## Components

| Component | Key Fields |
|---|---|
| `Position` | `float x, y` |
| `Velocity` | `float vx, vy` |
| `Sprite` | `texturePath`, `currentFrame`, `totalFrames`, `animationSpeed` |
| `Health` | `currentHP`, `maxHP`, `invincible` |
| `Collider` | `width`, `height`, `tag` (`"player"`, `"enemy"`, `"projectile"`) |
| `Weapon` | `fireRate`, `cooldownTimer`, `damage`, `projectileType` |
| `NetworkId` | `uint32_t id` |
| `Score` | player score value |
| `Collectable` | power-up type and effect |

---

## System Execution Order

Each game tick runs systems in this order:

```
InputSystem → MovementSystem → CollisionSystem → HealthSystem
    → AnimationSystem → RenderSystem → NetworkSystem
```

On the server, `RenderSystem` is skipped.

---

## Game Module: State Machine

The game uses a state stack managed by `StateManager`. Each state implements:

```cpp
struct GameState {
    virtual void onEnter()  = 0;
    virtual void onExit()   = 0;
    virtual void update(float dt) = 0;
    virtual void render()   = 0;
};
```

States:

| State | Purpose |
|---|---|
| `MainMenuState` | Main menu (play, multiplayer, settings, quit) |
| `MultiplayerMenuState` | Room list, create/join room, chat |
| `PlayState` | Solo gameplay |
| `NetworkPlayState` | Multiplayer gameplay (receives server snapshots) |
| `SettingsState` | Volume, controls, saved to `assets/config/user_settings.json` |
| `GameOverState` | Game over screen |
| `VictoryState` | Victory screen |

---

## Network Data Flow

### Client side

1. `InputSystem` captures keyboard state and builds an 8-bit input mask.
2. `NetworkManager` serializes it as a `CLIENT_INPUT` packet and sends it to the server via UDP.
3. On receiving a `WORLD_SNAPSHOT`, the client updates ECS component data for all entities.
4. Positions are interpolated between the last and current received states before rendering.

### Server side

1. Server receives `CLIENT_INPUT` packets and queues them per session.
2. Each tick: dequeue inputs → validate → apply to ECS → run all systems.
3. Every 100 ms: serialize all active entity states into a `WORLD_SNAPSHOT` and broadcast to all clients in the room.

### Delta compression

The server caches the last sent `EntityState` per entity per room. A delta is only sent if:
- position delta > 0.05
- velocity delta > 0.01
- any integer field (HP, type, etc.) changed

A full snapshot is always forced on `GAME_START`.

Result: ~33% reduction in outbound bandwidth during active gameplay.
