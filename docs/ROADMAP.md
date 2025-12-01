# 🗺️ Roadmap Personnelle - Game Engine R-Type

> Ton plan d'action semaine par semaine pour implémenter le game engine

---

## 📅 Timeline Globale

```
Semaine 1-2 : Fondations ECS + Core
Semaine 3   : Rendering + Network Abstraction
Semaine 4   : Prototype jouable (DEADLINE PART 1)
Semaine 5-7 : Features avancées (DEADLINE PART 2)
```

---

## 🎯 Semaine 1 : ECS Foundations

### Objectif
Avoir un système ECS fonctionnel avec entities, components, et systems basiques.

### Tasks
- [ ] **Jour 1-2 : Entity Management**
  - [ ] Créer `engine/include/engine/ecs/Entity.hpp`
    ```cpp
    using Entity = uint32_t;
    constexpr Entity NULL_ENTITY = 0;
    ```
  - [ ] Implémenter `EntityManager` (create, destroy, isAlive)
  - [ ] Système de génération pour détecter entities invalides
  - [ ] Tests unitaires

- [ ] **Jour 3-4 : Component Storage**
  - [ ] Template `ComponentManager<T>`
  - [ ] Implémentation `SparseSet<T>` ou `PackedArray<T>`
  - [ ] add/remove/get/has operations
  - [ ] Tests avec différents types de components

- [ ] **Jour 5 : Registry Base**
  - [ ] Classe `Registry` avec:
    - createEntity/destroyEntity
    - addComponent/getComponent/hasComponent/removeComponent
    - Map de ComponentManagers par type
  - [ ] Tests d'intégration

- [ ] **Jour 6-7 : View System**
  - [ ] Template `View<Components...>` pour queries
  - [ ] Itération sur entities avec components spécifiques
  - [ ] Optimisation (caching, etc.)

### Validation
```cpp
Registry reg;
Entity e = reg.createEntity();
reg.addComponent<Position>(e, {100, 200});
reg.addComponent<Velocity>(e, {10, 0});

auto view = reg.view<Position, Velocity>();
for (auto entity : view) {
    auto& pos = reg.getComponent<Position>(entity);
    auto& vel = reg.getComponent<Velocity>(entity);
    // ✅ Ça marche !
}
```

---

## 🎯 Semaine 2 : Systems + Core Utilities

### Objectif
Avoir des systems fonctionnels et les utilitaires de base.

### Tasks
- [ ] **Jour 1-2 : System Infrastructure**
  - [ ] Interface `ISystem` avec `update(Registry&, float dt)`
  - [ ] `SystemManager` : addSystem, removeSystem, update
  - [ ] Ordre d'exécution des systems
  - [ ] Premier system : `MovementSystem`

- [ ] **Jour 3 : Time Management**
  - [ ] Classe `Time` (deltaTime, totalTime)
  - [ ] Fixed timestep (optionnel)
  - [ ] Intégration dans la game loop

- [ ] **Jour 4 : Logger**
  - [ ] Classe `Logger` (info, warning, error, debug)
  - [ ] Multiple outputs (console, fichier)
  - [ ] Niveaux de log

- [ ] **Jour 5 : Event System**
  - [ ] `EventBus` (publish/subscribe pattern)
  - [ ] Template events
  - [ ] Tests avec différents types d'events

- [ ] **Jour 6-7 : Resource Manager + Config**
  - [ ] Template `ResourceManager<T>`
  - [ ] Cache de ressources
  - [ ] `Config` pour fichiers de configuration (JSON/TOML)

### Validation
```cpp
Registry reg;
reg.addSystem<MovementSystem>();

Entity e = reg.createEntity();
reg.addComponent<Position>(e, {0, 0});
reg.addComponent<Velocity>(e, {10, 0});

reg.update(1.0f); // 1 seconde

auto& pos = reg.getComponent<Position>(e);
// ✅ pos.x devrait être 10
```

---

## 🎯 Semaine 3 : Rendering + Network

### Objectif
Affichage graphique + communication réseau basique.

### Tasks
- [ ] **Jour 1-2 : Graphics Abstraction**
  - [ ] Interface `IRenderer` (clear, draw, present)
  - [ ] Interface `ITexture`, `ISprite`
  - [ ] Implémentation SFML : `SFMLRenderer`
  - [ ] `Window` management

- [ ] **Jour 2-3 : Render System**
  - [ ] `RenderSystem` : itère sur Sprite + Position
  - [ ] Support des layers de rendu
  - [ ] `Camera` basique (position, zoom)
  - [ ] Test : afficher un sprite

- [ ] **Jour 4 : Starfield Background**
  - [ ] Component `Parallax` ou `ScrollingBackground`
  - [ ] System pour scroll infini
  - [ ] Test : starfield animé

- [ ] **Jour 5-6 : Network Abstraction**
  - [ ] Classe `Packet` (write/read primitives)
  - [ ] Interface `ISocket`
  - [ ] `UDPSocket` avec Boost.Asio
  - [ ] Tests send/receive localhost

- [ ] **Jour 7 : Connection Management**
  - [ ] Classe `Connection` (état, adresse)
  - [ ] `ConnectionManager` (liste de connexions)
  - [ ] Heartbeat basique

### Validation
- ✅ Fenêtre s'ouvre
- ✅ Starfield s'affiche et scroll
- ✅ Sprite de vaisseau s'affiche
- ✅ Client peut envoyer/recevoir packets

---

## 🎯 Semaine 4 : Prototype Jouable (DEADLINE)

### Objectif
**PROTOTYPE JOUABLE pour la démo de fin de semaine 4**

### Tasks
- [ ] **Jour 1 : Input System**
  - [ ] `InputManager` (keyboard, mouse)
  - [ ] Component `PlayerInput`
  - [ ] Relier input → Velocity

- [ ] **Jour 2 : Collision System**
  - [ ] `CollisionDetector` (AABB)
  - [ ] `CollisionSystem`
  - [ ] Events de collision
  - [ ] Test : bullets hit enemies

- [ ] **Jour 3 : Gameplay Systems**
  - [ ] `WeaponSystem` (spawn bullets)
  - [ ] `HealthSystem` (damage, death)
  - [ ] Component `Lifetime` pour bullets

- [ ] **Jour 4 : Enemy Spawning**
  - [ ] `SpawnSystem` (enemies pattern)
  - [ ] AI basique (mouvement en vague)
  - [ ] Test : enemies apparaissent

- [ ] **Jour 5-6 : Network Integration**
  - [ ] Client envoie input au serveur
  - [ ] Serveur broadcast world state
  - [ ] Client affiche autres joueurs
  - [ ] Test multi-joueurs

- [ ] **Jour 7 : Polish + Démo**
  - [ ] Bug fixes
  - [ ] Performance checks
  - [ ] Préparation démo
  - [ ] Documentation mise à jour

### Validation ✅ PROTOTYPE
- [x] 2 joueurs peuvent se connecter
- [x] Vaisseaux se déplacent (WASD)
- [x] Tir de missiles (Space)
- [x] Enemies apparaissent et bougent
- [x] Collisions fonctionnent
- [x] Starfield background
- [x] Game over quand health = 0

---

## 🎯 Semaine 5-7 : Advanced Features

### Objectif
Polish + features avancées pour la version finale.

### Tasks Prioritaires
- [ ] **Animation System**
  - [ ] Component `Animator` (frames, timing)
  - [ ] `AnimationSystem`
  - [ ] Sprites animés (explosion, vaisseau)

- [ ] **Particle System**
  - [ ] Component `ParticleEmitter`
  - [ ] `ParticleSystem`
  - [ ] Effets : explosion, thrust

- [ ] **Network Improvements**
  - [ ] Client-side prediction
  - [ ] Server reconciliation
  - [ ] Interpolation (smooth movement)
  - [ ] Delta compression (optionnel)

- [ ] **AI Improvements**
  - [ ] Patterns de mouvement complexes
  - [ ] Boss fight (optionnel)
  - [ ] Différents types d'enemies

- [ ] **Audio System** (optionnel)
  - [ ] `AudioManager`
  - [ ] Sound effects (tir, explosion)
  - [ ] Musique background

### Tasks Optionnelles
- [ ] Spatial partitioning (QuadTree)
- [ ] Object pooling (performance)
- [ ] Scripting (Lua)
- [ ] Replay system
- [ ] Profiling tools

### Validation ✅ FINAL
- [x] Animations fluides
- [x] Particules (explosions)
- [x] Network stable (pas de lag visible)
- [x] 4+ joueurs simultanés
- [x] Multiple enemy types
- [x] Polish graphique
- [x] Documentation complète
- [x] Tests unitaires >80% coverage

---

## 📊 Métriques de Progression

### ECS Core
```
[████████████████████████░░░░] 80%
✅ Entity, Component, Registry
✅ Systems, View
⏳ Advanced queries
```

### Rendering
```
[████████████░░░░░░░░░░░░░░░░] 40%
✅ Window, Renderer
✅ Basic sprites
⏳ Animations, Particles
```

### Network
```
[████████░░░░░░░░░░░░░░░░░░░░] 30%
✅ Packet, Socket
⏳ Prediction, Interpolation
```

### Game Logic
```
[████████████░░░░░░░░░░░░░░░░] 40%
✅ Movement, Collision
⏳ Advanced AI, Weapons
```

---

## 🚨 Points de Contrôle

### Checkpoint 1 (Fin Semaine 2)
**Questions** :
- [ ] Peux-tu créer 1000 entities avec components en <1ms ?
- [ ] Les systems s'exécutent dans le bon ordre ?
- [ ] Les tests unitaires passent ?

### Checkpoint 2 (Fin Semaine 3)
**Questions** :
- [ ] Affichage fonctionne à 60 FPS ?
- [ ] Client/serveur peuvent communiquer ?
- [ ] ResourceManager cache correctement ?

### Checkpoint 3 (Fin Semaine 4) - DÉMO
**Questions** :
- [ ] Le prototype est jouable ?
- [ ] Multiplayer fonctionne ?
- [ ] Aucun crash ?

### Checkpoint 4 (Fin Semaine 7) - FINAL
**Questions** :
- [ ] Toutes les features sont implémentées ?
- [ ] Documentation complète ?
- [ ] Code review fait ?

---

## 📝 Daily Checklist Template

Utilise cette checklist **chaque jour** :

```markdown
## Date : __/__/____

### Morning (9h-12h)
- [ ] Review : relire code d'hier
- [ ] Plan : définir objectifs du jour
- [ ] Code : implémenter feature X
- [ ] Test : écrire tests unitaires

### Afternoon (14h-18h)
- [ ] Code : continuer feature X
- [ ] Debug : corriger bugs
- [ ] Commit : git commit avec message clair
- [ ] Doc : mettre à jour documentation

### End of Day
- [ ] Code review avec l'équipe
- [ ] Update roadmap (cocher items)
- [ ] Plan demain
- [ ] Push to git

### Notes
- Problèmes rencontrés : ...
- Solutions trouvées : ...
- Questions pour demain : ...
```

---

## 🎓 Apprentissage Continu

### Chaque semaine, lis :
- 1 article sur l'ECS
- 1 article sur le game networking
- 1 section du livre "Game Engine Architecture"

### Ressources hebdomadaires :
**Semaine 1** : ECS basics
- https://skypjack.github.io/2019-02-14-ecs-baf-part-1/

**Semaine 2** : ECS advanced
- https://www.youtube.com/watch?v=W3aieHjyNvw (Overwatch ECS)

**Semaine 3** : Networking
- https://gafferongames.com/post/introduction_to_networked_physics/

**Semaine 4** : Optimization
- https://www.dataorienteddesign.com/dodbook/

---

## 💡 Tips & Tricks

### Productivité
- 🍅 **Pomodoro** : 25min code, 5min pause
- 📝 **Documentation as you go** : ne reporte pas
- 🧪 **Test-driven** : tests avant implémentation
- 🔄 **Refactor souvent** : garde le code clean

### Debugging
- 🐛 Use `Logger` partout
- 🔍 Visual debugger (gdb, lldb)
- 📊 Profiler (Valgrind, perf)
- 🎯 Reproduce bugs in unit tests

### Git Workflow
```bash
# Feature branch
git checkout -b feature/ecs-registry
# Commits atomiques
git commit -m "feat(ecs): add Registry::createEntity()"
# Rebase avant merge
git rebase main
# Merge
git checkout main && git merge feature/ecs-registry
```

---

## 🏆 Success Criteria

Tu as réussi le game engine si :

✅ **Fonctionnel**
- Prototype jouable semaine 4
- Version finale semaine 7
- Multiplayer stable

✅ **Architecture**
- ECS bien implémenté
- Abstractions claires
- Code réutilisable

✅ **Documentation**
- README complet
- Dev docs à jour
- Protocol documenté

✅ **Qualité**
- Tests unitaires >70%
- Pas de memory leaks
- 60 FPS stable

✅ **Teamwork**
- Code reviews réguliers
- Git workflow propre
- Communication claire

---

**Bon courage ! Tu vas assurer ! 🚀💪**

N'oublie pas : **Fais marcher avant d'optimiser.**
