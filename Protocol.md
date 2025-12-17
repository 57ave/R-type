# 📘 R-Type — Protocol Documentation

Ce document décrit le **protocole binaire UDP** utilisé pour la communication entre le serveur et les clients R-Type.

Le protocole est conçu pour être :

* **Rapide** (UDP, binaire compact)
* **Robuste** (détection de pertes, duplications, hors séquence)
* **Sûr** (jamais faire crash client/serveur)
* **Documenté** (quelqu’un peut écrire un nouveau client juste avec ce document)

---

# 📡 1. Transport Layer

## 🚀 UDP obligatoire

Toutes les communications en jeu utilisent **exclusivement UDP**.

Avantages :

* très rapide
* pas de connexion lourde
* accepté pour les jeux temps réel

Inconvénients pris en charge par le protocole :

* pertes de paquets
* duplication
* désordre dans l’ordre d’arrivée

## ☑ TCP optionnel

Uniquement autorisé pour :

* login / lobby
* téléchargement de ressources
* debug

Mais *pas obligatoire*.

---

# 🧱 2. Structure générale d'un paquet UDP

Tous les paquets commencent par les champs suivants :

```
struct PacketHeader {
    uint16_t magic;      // 0x5254 ('RT') signature
    uint8_t  version;    // version du protocole
    uint8_t  type;       // type du paquet (enum PacketType)
    uint32_t seq;        // numéro de séquence (anti-duplication)
    uint32_t timestamp;  // ms depuis start du serveur
};
```

### 📌 Exploitation des champs

* `magic` → vérifie que le paquet appartient à ce jeu
* `version` → incompatible = rejet propre
* `type` → identifie la forme du payload
* `seq` → détection du désordre / duplications
* `timestamp` → latence, interpolation client

---

# 🔤 3. Types de paquets (`PacketType`)

Voici **l’ensemble minimal** des paquets nécessaires.

## 🔼 3.1 Client → Serveur

| ID   | Nom                 | Description           |
| ---- | ------------------- | --------------------- |
| 0x01 | `CLIENT_HELLO`      | demande de connexion  |
| 0x02 | `CLIENT_INPUT`      | input du joueur       |
| 0x03 | `CLIENT_PING`       | ping → test connexion |
| 0x04 | `CLIENT_DISCONNECT` | départ propre         |

### `CLIENT_INPUT`

Payload :

```
struct ClientInput {
    uint8_t playerId;
    uint8_t inputMask;  // 1 bit par action
};
```

Input mask :

| Bit | Action     |
| --- | ---------- |
| 0   | Move Up    |
| 1   | Move Down  |
| 2   | Move Left  |
| 3   | Move Right |
| 4   | Fire       |
| 5-7 | réservés   |

---

## 🔽 3.2 Serveur → Client

| ID   | Nom                 | Description                   |
| ---- | ------------------- | ----------------------------- |
| 0x10 | `SERVER_WELCOME`    | confirmation de connexion     |
| 0x11 | `WORLD_SNAPSHOT`    | état complet du monde         |
| 0x12 | `ENTITY_SPAWN`      | nouvelle entité               |
| 0x13 | `ENTITY_DESTROY`    | destruction d’entité          |
| 0x14 | `PLAYER_DIED`       | un joueur est mort            |
| 0x15 | `SERVER_PING_REPLY` | réponse au ping               |
| 0x16 | `CLIENT_LEFT`       | informe qu’un client a quitté |

### `WORLD_SNAPSHOT`

C’est le gros paquet envoyé **60 fois par seconde**.

Payload :

```
struct SnapshotHeader {
    uint32_t entityCount;
};

struct EntityState {
    uint32_t id;
    uint8_t  type;      // enum EntityType
    float    x;
    float    y;
    float    vx;
    float    vy;
    uint8_t  hp;        // 0 = mort
};
```

---

# 🎯 4. Types d’entités

```
enum EntityType : uint8_t {
    ENTITY_PLAYER = 0,
    ENTITY_MONSTER = 1,
    ENTITY_PLAYER_MISSILE = 2,
    ENTITY_MONSTER_MISSILE = 3,
    ENTITY_OBSTACLE = 4,
};
```

Chaque client sait comment **afficher** un type d’entité.

---

# 📦 5. Sérialisation binaire

Le protocole utilise :

* little endian
* padding interdit
* structures compactes
* types fixes (`uint8_t`, `uint16_t`, etc.)

Toutes les structures sont **packées** :

```
#pragma pack(push, 1)
struct ...
#pragma pack(pop)
```

---

# 📉 6. Gestion des erreurs & robustesse

Le serveur **ne doit jamais** crash sur :

* paquet trop court
* type inconnu
* version incompatible
* payload tronqué ou incohérent
* entité inexistante

Comportement correct :

* log
* ignorer
* continuer

Le client pareil.

---

# 📐 7. Fiabilisation du protocole (UDP)

Stratégies intégrées :

### 📌 Numérotation (`seq`)

Permet de :

* ignorer les doublons
* rejeter les paquets trop vieux
* estimer la latence

### 📌 Keep Alive

Le client envoie un `CLIENT_PING` toutes les 500 ms.
Le serveur répond avec `SERVER_PING_REPLY`.

### 📌 Timeout

Si un client n’a rien envoyé depuis 5 secondes :

* enlever le client
* informer les autres (`CLIENT_LEFT`)

---

# ⏱ 8. Fréquences d’envoi

| Paquet           | Fréquence                        |
| ---------------- | -------------------------------- |
| `CLIENT_INPUT`   | à chaque frame locale (20–60 Hz) |
| `WORLD_SNAPSHOT` | 60 Hz (tickrate serveur)         |
| `ENTITY_SPAWN`   | évènement instantané             |
| `ENTITY_DESTROY` | évènement instantané             |
| `PLAYER_DIED`    | évènement instantané             |
| `PING`           | 500 ms                           |

---

# 🔍 9. Exemples de flux

## 🔼 Connexion

1. client → `CLIENT_HELLO`
2. serveur → `SERVER_WELCOME`
3. client → `CLIENT_INPUT` en boucle
4. serveur → `WORLD_SNAPSHOT 60Hz`

## 🔽 Déplacement

1. client → `CLIENT_INPUT(mask=LEFT)`
2. serveur → applique
3. serveur → `WORLD_SNAPSHOT`
4. client interpole

---