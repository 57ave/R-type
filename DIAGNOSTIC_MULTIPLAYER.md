# 🔧 Diagnostic des Problèmes Multiplayer

## 📋 Résumé Exécutif

**Problème Principal** : Vous avez lancé le jeu sans l'argument `--network`, donc il a démarré en mode solo local au lieu de se connecter au serveur.

## ❌ Ce qui s'est passé

### Vos commandes :
```bash
./build/server/r-type_server > output_test/server.log
./build/game/r-type_game > output_test/player1.log  # ❌ ERREUR ICI !
```

### Résultat dans les logs :
```
[Game] Local mode (use --network <ip> <port> for multiplayer)
[Game] *** isNetworkClient = FALSE ***
```

## ✅ Solution

### Commandes CORRECTES :
```bash
# Terminal 1 : Serveur
./build/server/r-type_server > output_test/server.log

# Terminal 2 : Client 1 (avec --network !)
./build/game/r-type_game --network 127.0.0.1 12345 > output_test/player1.log

# Terminal 3 : Client 2 (avec --network !)
./build/game/r-type_game --network 127.0.0.1 12345 > output_test/player2.log
```

### Ou utilisez le script automatique :
```bash
./test_multiplayer_simple.sh
```

## 🔍 Diagnostic Détaillé

### 1. Un seul joueur visible
**Cause** : Le jeu n'est PAS en mode réseau  
**Raison** : Manque `--network 127.0.0.1 12345`  
**Impact** : Le client crée son propre joueur local sans se connecter au serveur

### 2. Gameplay local au lieu de multiplayer
**Cause** : Même cause  
**Raison** : `isNetworkClient = FALSE` dans le code  
**Impact** : Le client gère son propre gameplay (ennemis, collisions) au lieu de recevoir les données du serveur

### 3. Visuels des ennemis incorrects (si en mode network)
**Cause** : Si réellement en mode network, les sprites sont créés dynamiquement  
**Code** : `Game.cpp` lignes 1015-2200 gèrent la création de sprites pour les entités réseau  
**Status** : ✅ Le code est CORRECT

## 📊 Flux de Données Multiplayer

```
Sans --network (MODE LOCAL) :
┌──────────┐
│  Client  │ ← Gère TOUT : joueur, ennemis, collisions
└──────────┘ ← Aucune connexion réseau

Avec --network (MODE MULTIPLAYER) :
┌──────────┐          ┌────────┐          ┌──────────┐
│ Client 1 │ ←──────→ │ Server │ ←──────→ │ Client 2 │
└──────────┘   Input  └────────┘   Input  └──────────┘
     ↑          ENTITY_SPAWN              ↑
     └──────────WORLD_SNAPSHOT────────────┘
```

## 🎯 Vérification Rapide

### Vérifiez les logs :

**✅ Bon (Network Mode)** :
```
[Game] Network mode enabled. Server: 127.0.0.1:12345
[Game] *** isNetworkClient = TRUE ***
[Game] Network client started, waiting for SERVER_WELCOME...
[Game] Connected! Player ID: 1
```

**❌ Mauvais (Local Mode)** :
```
[Game] Local mode (use --network <ip> <port> for multiplayer)
[Game] *** isNetworkClient = FALSE ***
```

## 🚀 Checklist de Lancement

- [ ] Compilé le projet : `cd build && cmake .. && make`
- [ ] Serveur lancé : `./build/server/r-type_server`
- [ ] Client 1 lancé **avec `--network 127.0.0.1 12345`**
- [ ] Client 2 lancé **avec `--network 127.0.0.1 12345`**
- [ ] Client 1 : Créé une room dans MULTIPLAYER
- [ ] Client 2 : Rejoint la room
- [ ] Client 1 : Cliqué sur START GAME
- [ ] Les deux joueurs sont visibles à l'écran

## 📁 Fichiers à Consulter

1. `MULTIPLAYER_GUIDE.md` - Guide complet d'utilisation
2. `test_multiplayer_simple.sh` - Script de lancement automatique
3. `run_multiplayer.sh` - Script interactif avec tmux

## 🛠️ Code Technique

### Argument --network dans le code

Fichier : `game/src/Game.cpp` (lignes 577-590)

```cpp
if (argc > 1 && std::string(argv[1]) == "--network") {
    networkMode = true;
    isNetworkClient = true;  // ← CRUCIAL !
    if (argc > 2) {
        serverAddress = argv[2];  // IP
    }
    if (argc > 3) {
        serverPort = static_cast<short>(std::stoi(argv[3]));  // Port
    }
    std::cout << "[Game] Network mode enabled. Server: " << serverAddress << ":" << serverPort << std::endl;
} else {
    std::cout << "[Game] Local mode (use --network <ip> <port> for multiplayer)" << std::endl;
    // ← Mode LOCAL activé car pas de --network
}
```

### Création des sprites réseau

Fichier : `game/src/Game.cpp` (lignes 1015-1170)

```cpp
networkSystem->setEntityCreatedCallback([this](ECS::Entity entity) {
    // Callback appelé quand le serveur envoie ENTITY_SPAWN
    
    auto& tag = gCoordinator.GetComponent<Tag>(entity);
    
    if (tag.name == "Player") {
        // Crée le sprite du joueur avec la bonne couleur
        auto& netId = gCoordinator.GetComponent<NetworkId>(entity);
        sprite->setTexture(playerTexture.get());
        IntRect rect(33 * 2, netId.playerLine * 17, 33, 17);
        // ...
    }
    else if (tag.name == "Enemy") {
        // Crée le sprite de l'ennemi
        sprite->setTexture(textureMap["enemy"]);
        IntRect rect(0, 0, 33, 32);
        // ...
    }
    // ... autres types d'entités
});
```

## 🎓 Conclusion

**Le code du jeu est CORRECT.**  
**Le problème était uniquement dans la façon de le lancer.**

Utilisez toujours `--network 127.0.0.1 12345` quand vous voulez jouer en multiplayer !
