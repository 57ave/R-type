# 🎮 Guide de Lancement Multiplayer R-Type

## ❌ Problème Identifié

Vous avez lancé le jeu **sans les arguments de connexion réseau**, ce qui a démarré le mode local au lieu du mode multiplayer.

### Ce que vous avez fait (INCORRECT) :
```bash
./build/server/r-type_server > output_test/server.log
./build/game/r-type_game > output_test/player1.log  # ❌ PAS DE --network !
```

**Résultat** : Le client démarre en mode local et crée son propre gameplay solo

### Log du problème :
```
[Game] Local mode (use --network <ip> <port> for multiplayer)
[Game] *** isNetworkClient = FALSE ***
```

---

## ✅ Solutions

### Option 1 : Script de Test Simple (RECOMMANDÉ)

Utilisez le script `test_multiplayer_simple.sh` :

```bash
./test_multiplayer_simple.sh
```

Ce script va :
1. ✅ Lancer le serveur sur le port 12345
2. ✅ Lancer 2 clients **avec l'argument `--network 127.0.0.1 12345`**
3. ✅ Créer des logs dans `output_test/`
4. ✅ Afficher les PIDs pour faciliter l'arrêt

### Option 2 : Script Interactif Complet

Utilisez le script original avec tmux :

```bash
./run_multiplayer.sh
```

Puis choisissez l'option **5** pour le mode recommandé.

### Option 3 : Lancement Manuel (3 terminaux)

**Terminal 1 - Serveur :**
```bash
./build/server/r-type_server
```

**Terminal 2 - Client 1 :**
```bash
./build/game/r-type_game --network 127.0.0.1 12345
```

**Terminal 3 - Client 2 :**
```bash
./build/game/r-type_game --network 127.0.0.1 12345
```

---

## 🎮 Workflow de Jeu Multiplayer

Une fois les clients lancés **avec `--network`** :

1. **Client 1 (Host)** :
   - Menu principal → MULTIPLAYER
   - CREATE ROOM
   - Nommer la room
   - Attendre les joueurs

2. **Client 2 (Player 2)** :
   - Menu principal → MULTIPLAYER
   - SERVER BROWSER
   - Cliquer sur la room créée par Client 1
   - JOIN

3. **Client 1 (Host)** :
   - Quand tous les joueurs sont prêts
   - Cliquer sur START GAME

4. **Les deux clients** :
   - Le jeu démarre
   - ✅ Vous verrez **2 vaisseaux** (un pour chaque joueur)
   - ✅ Les ennemis seront synchronisés
   - ✅ Les mouvements seront partagés

---

## 🔍 Vérification que le mode réseau est actif

Dans les logs du client, vous devriez voir :

✅ **CORRECT (Mode Network)** :
```
[Game] Network mode enabled. Server: 127.0.0.1:12345
[Game] *** isNetworkClient = TRUE ***
[Game] Network client started, waiting for SERVER_WELCOME...
[Game] Connected! Player ID: 1
```

❌ **INCORRECT (Mode Local)** :
```
[Game] Local mode (use --network <ip> <port> for multiplayer)
[Game] *** isNetworkClient = FALSE ***
```

---

## 📝 Architecture du Système

```
┌─────────────────┐
│  r-type_server  │  ← Lance le gameplay, spawn les ennemis
│   Port 12345    │  ← Gère la physique du jeu
└────────┬────────┘  ← Broadcast les entités à tous les clients
         │
    ┌────┴────┐
    │         │
┌───▼──┐  ┌──▼───┐
│Client│  │Client│  ← Affichent les sprites
│  1   │  │  2   │  ← Envoient les inputs au serveur
└──────┘  └──────┘  ← Reçoivent les updates du serveur
```

### Flux de données :

1. **Client → Serveur** : Inputs (clavier, souris)
2. **Serveur → Clients** : 
   - ENTITY_SPAWN (joueurs, ennemis, projectiles)
   - WORLD_SNAPSHOT (positions)
   - ENTITY_DESTROY (mort d'entités)

---

## 🐛 Problèmes Connus et Solutions

### Problème 1 : Un seul joueur visible
**Cause** : Jeu lancé sans `--network`  
**Solution** : Utilisez `./test_multiplayer_simple.sh`

### Problème 2 : Gameplay local au lieu de multiplayer
**Cause** : Même cause que Problème 1  
**Solution** : Même solution

### Problème 3 : Visuels des ennemis incorrects
**Cause** : Si le jeu est bien en mode network, c'est un problème de synchronisation  
**Solution** : Le code crée automatiquement les sprites pour les entités réseau (voir Game.cpp lignes 1960-2100)

---

## 🚀 Commande Rapide

Pour démarrer un test complet :

```bash
# Tout en un (recommandé)
./test_multiplayer_simple.sh

# OU avec tmux (plus avancé)
./run_multiplayer.sh
# Puis choisir l'option 4 ou 5
```

---

## 📊 Logs de Débogage

Les logs sont créés dans `output_test/` :
- `server.log` : Log du serveur
- `player1.log` : Log du client 1
- `player2.log` : Log du client 2

Vérifiez toujours que les clients affichent `isNetworkClient = TRUE` !

---

## ⚠️ IMPORTANT

**Ne jamais lancer le client sans `--network` si vous voulez jouer en multijoueur !**

Le mode local et le mode network sont **mutuellement exclusifs** :
- **Sans `--network`** : Gameplay solo local géré par le client
- **Avec `--network`** : Gameplay multiplayer géré par le serveur
