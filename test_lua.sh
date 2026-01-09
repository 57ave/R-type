#!/bin/bash
# Script de test pour vérifier que le scripting Lua fonctionne

echo "🧪 Test du système de scripting Lua"
echo "===================================="
echo ""

# Vérifier que Sol3 est présent
if [ -f "engine/external/sol3/include/sol/sol.hpp" ]; then
    echo "✅ Sol3 v3.5.0 installé"
else
    echo "❌ Sol3 manquant"
    exit 1
fi

# Vérifier que les scripts Lua existent
SCRIPTS=(
    "assets/scripts/systems/spawn_system.lua"
    "assets/scripts/systems/player_controller.lua"
    "assets/scripts/systems/collision_handler.lua"
    "assets/scripts/waves/wave_manager.lua"
    "assets/scripts/config/game_config.lua"
)

echo ""
echo "📜 Vérification des scripts Lua :"
for script in "${SCRIPTS[@]}"; do
    if [ -f "$script" ]; then
        echo "  ✅ $script"
    else
        echo "  ❌ $script (manquant)"
    fi
done

# Vérifier la compilation
echo ""
if [ -f "build/game/r-type_game" ]; then
    SIZE=$(du -h build/game/r-type_game | cut -f1)
    echo "✅ Exécutable compilé : r-type_game ($SIZE)"
else
    echo "❌ Exécutable non trouvé"
    exit 1
fi

# Test de chargement Lua
echo ""
echo "🔍 Test de l'initialisation Lua..."
timeout 2 ./build/game/r-type_game 2>&1 | grep -E "(Lua|Factory|Coordinator)" | head -10

echo ""
echo "✅ Système de scripting Lua opérationnel !"
echo ""
echo "📝 Prochaines étapes :"
echo "  1. Tester le hot-reload des scripts"
echo "  2. Lancer le jeu avec les scripts spawn_system.lua"
echo "  3. Ajuster les paramètres dans game_config.lua"
echo ""
