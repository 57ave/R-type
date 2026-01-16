# 🔍 AUDIT: Code R-Type spécifique restant dans l'Engine

## Résultat de l'audit (12 janvier 2026)

### ✅ Ce qui est maintenant générique

1. **Components génériques** (engine/include/components/)
   - ✅ Position, Velocity, Sprite, Animation
   - ✅ Health (avec `deathEffect` string-based)
   - ✅ Collider, Boundary
   - ✅ Lifetime (Effect déplacé vers module)
   - ✅ Tag (string-based, 100% générique)
   - ✅ ScrollingBackground (utile pour tous genres)

2. **Module shootemup** (engine/modules/shootemup/)
   - ✅ Tous les components sont string-based (Weapon, MovementPattern, Attachment, etc.)
   - ✅ Pas d'enums hardcodés
   - ✅ Configuration via Lua possible

### ❌ Code ENCORE trop spécifique à R-Type

#### 🚨 CRITIQUE: Réseau (engine/include/network/)

**Fichier:** `RTypeProtocol.hpp`
**Problème:** Protocole réseau COMPLÈTEMENT spécifique à R-Type

```cpp
// ❌ Ces enums sont R-Type specific:
enum class EntityType : uint8_t {
    ENTITY_PLAYER = 0,           // ❌ "Player" concept
    ENTITY_MONSTER = 1,          // ❌ "Monster" (not "Enemy")
    ENTITY_PLAYER_MISSILE = 2,   // ❌ Missiles R-Type
    ENTITY_MONSTER_MISSILE = 3,  // ❌ Enemy missiles
    ENTITY_OBSTACLE = 4,
    ENTITY_EXPLOSION = 5
};

// ❌ Structure avec fields R-Type specific:
struct EntityState {
    uint8_t playerLine;      // ❌ R-Type spritesheet specific
    uint8_t chargeLevel;     // ❌ R-Type charge shot mechanic
    uint8_t enemyType;       // ❌ Enemy types
    uint8_t projectileType;  // ❌ Projectile types
};

// ❌ Packet types R-Type specific:
enum class GamePacketType : uint16_t {
    PLAYER_DIED = 0x14,      // ❌ "Player" concept
    // ...
};
```

**Impact:** Ce protocole ne peut être utilisé QUE pour R-Type, impossible de faire un autre jeu avec!

**Solution recommandée:**
1. **Déplacer** `RTypeProtocol.hpp` vers `game/include/network/` ou `server/include/`
2. **Créer** un protocole générique dans l'engine avec:
   - String-based entity types
   - Flexible attribute system (key-value pairs)
   - Game-agnostic packet types

---

**Fichier:** `NetworkClient.hpp`, `NetworkServer.hpp`
**Problème:** Dépendent de RTypeProtocol.hpp

```cpp
// ❌ Hardcoded pour R-Type
void sendInput(uint8_t playerId, uint8_t inputMask, uint8_t chargeLevel = 0);
uint8_t getPlayerId() const;
```

**Solution:** Abstraction générique avec templates ou type erasure

---

#### ⚠️ MOYEN: Input System (engine/include/systems/)

**Fichier:** `InputSystem.hpp` (DÉJÀ CORRIGÉ ✅)
- ~~Avait des inputs hardcodés (SHOOT, BOMB)~~
- ✅ Maintenant string-based ("move_up", "action1", etc.)

---

**Fichier:** `StateMachineAnimationSystem.hpp`
**Problème:** Documentation mentionne "player ships" avec tilt up/down

```cpp
/**
 * @brief StateMachineAnimationSystem - Handles state-based animations for player ships
 * Used for player ships that tilt up/down/neutral
 */
```

**Impact:** Faible (c'est juste la doc, le code peut être générique)

**Solution:** Renommer en `StateAnimationSystem` et rendre la doc générique

---

#### ℹ️ FAIBLE: Documentation

**Fichiers:** Plusieurs fichiers ont des commentaires mentionnant R-Type
- `Components.hpp`: Exemples avec "Player", "Enemy" dans les commentaires
- `ComponentBindings.hpp`: Mentionne "Player, Enemy, PowerUp"

**Impact:** Très faible (juste documentation/exemples)

**Solution:** Nettoyer les commentaires pour utiliser des exemples génériques

---

## 📊 Récapitulatif

| Catégorie | Status | Priorité | Effort |
|-----------|--------|----------|--------|
| Components core | ✅ Générique | - | Fait |
| Module shootemup | ✅ Générique | - | Fait |
| Input System | ✅ Générique | - | Fait |
| **Network Protocol** | ❌ R-Type specific | 🔴 HAUTE | 🔧 Moyen |
| **Network Client/Server** | ❌ R-Type specific | 🔴 HAUTE | 🔧 Moyen |
| State Animation System | ⚠️ Doc R-Type | 🟡 Moyenne | 🔧 Faible |
| Documentation | ⚠️ Exemples R-Type | 🟢 Basse | 🔧 Faible |

---

## 🎯 Plan d'action recommandé

### Phase 1: Réseau (PRIORITÉ HAUTE)

**Option A: Déplacer hors de l'engine** (⏱️ 15 min, ⭐ Recommandé)
```bash
# Déplacer le protocole R-Type vers game/
mv engine/include/network/RTypeProtocol.hpp game/include/network/
mv engine/include/network/ClientSession.hpp game/include/network/

# Mettre à jour les includes dans game/ et server/
```

**Avantages:**
- ✅ Rapide
- ✅ Engine devient immédiatement réutilisable
- ✅ Pas besoin de refactoriser le réseau existant

**Inconvénients:**
- ❌ Pas de réseau générique dans l'engine (mais peut être ajouté plus tard)

---

**Option B: Créer protocole générique** (⏱️ 2-3h, 🚀 Meilleur long terme)

Créer `engine/include/network/GenericProtocol.hpp`:
```cpp
// Generic entity representation
struct GenericEntityState {
    uint32_t id;
    std::string type;                    // "player", "enemy", "projectile", etc.
    float x, y, vx, vy;
    std::map<std::string, float> attrs;  // Flexible attributes
};

// Generic packet types
enum class PacketType : uint16_t {
    CLIENT_CONNECT = 0x01,
    CLIENT_ACTION = 0x02,
    SERVER_WELCOME = 0x10,
    WORLD_STATE = 0x11,
    ENTITY_EVENT = 0x12
};
```

**Avantages:**
- ✅ Engine complètement réutilisable
- ✅ Supporte N'IMPORTE QUEL jeu multijoueur
- ✅ Architecture propre

**Inconvénients:**
- ❌ Plus de travail
- ❌ Besoin de migrer game/ et server/

---

### Phase 2: Nettoyage documentation (PRIORITÉ BASSE)

1. Remplacer exemples "Player"/"Enemy" par "Entity A"/"Entity B"
2. Généraliser les commentaires dans Systems
3. Renommer `StateMachineAnimationSystem` → `StateAnimationSystem`

---

## 🎮 Conclusion

**État actuel:**
- ✅ **Engine ECS**: 100% générique
- ✅ **Components**: 100% génériques
- ✅ **Module shootemup**: 100% réutilisable avec configs Lua
- ❌ **Réseau**: 100% R-Type specific ← **BLOQUANT** pour réutilisabilité

**Recommandation immédiate:**
👉 **Déplacer `RTypeProtocol.hpp` et fichiers réseau vers `game/` ou `server/`**

Cela rendra l'engine **immédiatement** réutilisable pour d'autres jeux (single-player ou avec leur propre protocole réseau).

Le multijoueur générique peut être ajouté plus tard si nécessaire.

---

## 📝 Checklist finale

Pour avoir un engine 100% abstrait et réutilisable:

- [x] Components génériques (Position, Velocity, Sprite, etc.)
- [x] Tag system string-based
- [x] Health avec deathEffect configurable
- [x] Effect déplacé vers module shootemup
- [x] InputSystem string-based
- [x] Module shootemup 100% configurable via Lua
- [ ] **Protocole réseau déplacé hors de l'engine** ← **À FAIRE**
- [ ] Documentation nettoyée (optionnel)
- [ ] StateAnimationSystem renommé (optionnel)

**Une fois le réseau déplacé, l'engine sera 100% générique! 🎉**
