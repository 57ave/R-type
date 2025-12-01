# Documentation du Game Engine - R-Type

Bienvenue dans la documentation du game engine pour le projet R-Type.

## 📚 Documents disponibles

### 1. **[ENGINE_IMPLEMENTATION_GUIDE.md](./ENGINE_IMPLEMENTATION_GUIDE.md)** 📖
**Guide complet d'implémentation du game engine**

Ce document contient :
- Vue d'ensemble de l'architecture ECS (Entity Component System)
- Détails de tous les subsystems à implémenter (Core, Rendering, Network, Physics)
- Abstractions nécessaires
- Plan d'implémentation par phases (Semaine 1 à 7)
- Intégration avec le client et le serveur
- Checklist complète

👉 **Commence par ce document pour comprendre l'architecture globale**

---

### 2. **[QUICK_REFERENCE.md](./QUICK_REFERENCE.md)** ⚡
**Guide de référence rapide**

Ce document contient :
- Ordre d'implémentation recommandé
- Checklist de développement détaillée
- Patterns de code (exemples concrets)
- Structure CMakeLists.txt
- Components typiques pour R-Type
- FAQ et pièges à éviter
- Tests de validation

👉 **Utilise ce document pendant le développement comme référence rapide**

---

### 3. **[engine_architecture.puml](./engine_architecture.puml)** 🏗️
**Diagramme PlantUML de l'architecture complète**

Ce diagramme montre :
- Structure complète du game engine
- Relations entre subsystems (ECS, Core, Network, Rendering, Physics)
- Séparation client/server
- Détails des classes principales (Registry, EntityManager, Systems, etc.)
- Flow de données

👉 **Visualise ce diagramme pour comprendre l'architecture globale**

Pour générer l'image :
```bash
# Avec PlantUML installé
plantuml engine_architecture.puml

# Ou en ligne
# Copie le contenu sur http://www.plantuml.com/plantuml/uml/
```

---

### 4. **[ecs_detailed.puml](./ecs_detailed.puml)** 🧩
**Diagramme détaillé de l'ECS**

Ce diagramme montre :
- Architecture détaillée de l'Entity Component System
- Implémentation de SparseSet
- Exemples de components et systems
- Flow de données dans l'ECS
- Patterns d'utilisation avec code

👉 **Utilise ce diagramme pour implémenter l'ECS**

---

### 5. **[client_server_flow.puml](./client_server_flow.puml)** 🔄
**Diagramme de flux client/serveur**

Ce diagramme de séquence montre :
- Initialisation client/serveur
- Game loop complet (client-side prediction, server authority, reconciliation)
- Types de packets échangés
- Flow de données réseau
- Gestion des déconnexions

👉 **Utilise ce diagramme pour comprendre le networking**

---

## 🚀 Par où commencer ?

### Pour comprendre le projet
1. Lis **ENGINE_IMPLEMENTATION_GUIDE.md** en entier (30 min)
2. Regarde les diagrammes **engine_architecture.puml** et **ecs_detailed.puml**
3. Parcours **QUICK_REFERENCE.md** pour voir les exemples de code

### Pour développer
1. Suis le plan d'implémentation dans **ENGINE_IMPLEMENTATION_GUIDE.md**
2. Utilise **QUICK_REFERENCE.md** comme référence pendant le code
3. Coche les items de la checklist au fur et à mesure
4. Réfère-toi aux diagrammes quand tu es bloqué

---

## 📋 Ordre de lecture recommandé

```
1. ENGINE_IMPLEMENTATION_GUIDE.md (Vue d'ensemble)
   ↓
2. ecs_detailed.puml (Comprendre l'ECS)
   ↓
3. engine_architecture.puml (Architecture complète)
   ↓
4. QUICK_REFERENCE.md (Patterns de code)
   ↓
5. client_server_flow.puml (Networking)
```

---

## 🎯 Résumé ultra-rapide

### Qu'est-ce que tu dois implémenter ?

Le **Game Engine** est composé de :

1. **ECS (Entity Component System)** 
   - Fondation de tout le système
   - Entités = IDs, Components = Data, Systems = Logic

2. **Core Subsystem**
   - Time, Logger, ResourceManager, EventBus, Config

3. **Rendering Subsystem** (client uniquement)
   - Abstraction graphics, Window, Camera, RenderSystem

4. **Network Subsystem**
   - Abstraction socket, Packet, Connection, Interpolation

5. **Physics Subsystem**
   - Collision detection, Spatial partitioning

6. **Common Systems**
   - Movement, Animation, Health, Weapon, AI

### Priorités

**Semaine 1-2** : ECS + Core
**Semaine 3** : Rendering + Network
**Semaine 4** : Physics + Prototype jouable
**Semaine 5-7** : Features avancées + Polish

---

## 🛠️ Outils utiles

### Visualiser les diagrammes PlantUML

**Option 1 : En ligne**
- http://www.plantuml.com/plantuml/uml/
- Copie-colle le contenu des fichiers .puml

**Option 2 : VS Code extension**
```bash
# Installe l'extension PlantUML pour VS Code
code --install-extension jebbs.plantuml
```

**Option 3 : CLI**
```bash
# Ubuntu/Debian
sudo apt-get install plantuml

# macOS
brew install plantuml

# Générer l'image
plantuml engine_architecture.puml
```

### Générer tous les diagrammes
```bash
cd docs
plantuml *.puml
# Génère des .png de tous les diagrammes
```

---

## ❓ Questions ?

Si tu as des questions pendant l'implémentation :

1. Cherche dans **QUICK_REFERENCE.md** → Section FAQ
2. Regarde les diagrammes pour visualiser
3. Relis la section concernée dans **ENGINE_IMPLEMENTATION_GUIDE.md**
4. Consulte les ressources externes recommandées

---

## ✅ Validation

Après avoir lu cette documentation, tu devrais pouvoir répondre :

- [ ] Qu'est-ce qu'une Entity ? Un Component ? Un System ?
- [ ] Quelle est la différence entre client et serveur dans l'utilisation du engine ?
- [ ] Quels sont les subsystems à implémenter ?
- [ ] Par quoi dois-je commencer (phase 1) ?
- [ ] Comment tester chaque phase ?

Si tu peux répondre à ces questions, tu es prêt à coder ! 🚀

---

**Bon courage pour l'implémentation !** 💪
