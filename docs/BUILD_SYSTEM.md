# Système de Build & Dépendances R-Type

Ce document explique le fonctionnement du système de build "Self-Contained" du projet R-Type, comment il gère les dépendances (Lua, Sol3, SFML) et comment générer une version distribuable du jeu.

## 1. Philosophie "Self-Contained"

**Pourquoi ?**
L'objectif est de permettre à n'importe quel développeur (ou CI/CD) de cloner le repo et de compiler le projet **sans avoir à installer manuellement des bibliothèques complexes** sur son système (comme Lua 5.4, Sol3, ou même SFML). Cela garantit :
- **Reproductibilité** : Tout le monde utilise exactement les mêmes versions des librairies.
- **Simplicité** : Pas de "DLL hell" ou de conflits de versions installées sur le système.
- **Portabilité** : Fonctionne sur Linux, Windows et macOS avec un minimum de pré-requis (CMake, Compilateur C++).

**Comment ?**
Nous utilisons **CPM (CMake Package Manager)**. C'est un petit script CMake qui :
1.  Vérifie si une dépendance est déjà disponible sur le système (via `find_package`).
2.  Si elle est absente, **il la télécharge automatiquement** depuis GitHub (code source).
3.  Il l'intègre directement dans le build du projet (comme si c'était votre propre code code).

## 2. Gestion des Dépendances

### Lua 5.4 & Sol3
Le fichier `engine/CMakeLists.txt` gère cela :
- **Vérification** : Il tente d'abord de trouver Lua sur le système.
- **Fallback CPM** : Si Lua n'est pas trouvé, CPM télécharge `epezent/lua` (une version "CMake-friendly" de Lua 5.4) et la compile en tant que librairie statique.
- **Sol3** : Idem pour Sol3 (binding C++/Lua), qui est téléchargé depuis GitHub et ajouté comme librairie "Header Only".

### SFML 2.6
SFML est également gérée par CPM si elle n'est pas trouvée, garantissant que le moteur graphique est toujours disponible.

## 3. Installation & Packaging

Le build ne se contente pas de compiler les exécutables. Il définit des cibles d'**installation** (`install targets`) pour créer un dossier propre contenant tout le nécessaire pour jouer.

Les commandes `install(TARGETS ...)` et `install(DIRECTORY ...)` dans les `CMakeLists.txt` indiquent à CMake :
- Où mettre les exécutables (`bin/`)
- Où mettre les librairies (`lib/`)
- Où mettre les assets (`bin/assets`)

## 4. Guide d'Utilisation

### Pré-requis
- CMake 3.15+
- Compilateur C++ (GCC, Clang, MSVC)
- *Optionnel* : Ninja (pour des builds plus rapides)

### Compiler et Installer

La procédure standard pour obtenir un jeu jouable est la suivante :

```bash
# 1. Configuration (Génération des Makefiles)
# -B build : Crée les fichiers de build dans le dossier 'build'
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Compilation
# --build build : Lance la compilation dans le dossier 'build'
# -j : Utilise tous les cœurs du CPU
cmake --build build -j

# 3. Installation
# --install build : Copie les fichiers finaux dans le dossier d'installation par défaut
# (Sur Linux, par défaut c'est /usr/local, mais on peut spécifier un dossier local avec --prefix)
cmake --install build --prefix ./install
```

### Résultat

Après l'étape 3, vous aurez un dossier `install/` structuré ainsi :

```text
install/
├── bin/
│   ├── r-type_game       # Client du jeu
│   ├── r-type_server     # Serveur dédié
│   └── assets/           # Sprites, sons, scripts (copiés automatiquement)
└── lib/                  # Librairies statiques/dynamiques
```

Ce dossier `install` est **autonome**. Vous pouvez le zippé et l'envoyer à un autre utilisateur (sur le même OS), il pourra jouer directement.

### Pour les Développeurs (Test Rapide)
Vous pouvez toujours lancer les exécutables depuis le dossier `build`, mais rappelez-vous que pour le chargement des assets, le jeu s'attend généralement à trouver le dossier `assets/` à côté de l'exécutable ou dans le dossier de travail courant. L'étape d'installation règle ce problème définitivement.
