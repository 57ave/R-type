-- ==========================================
-- R-Type Game - Init Script
-- ==========================================
-- Ce script est chargé au démarrage du jeu
-- Il initialise tous les systèmes et charge les configs

print("[LUA] 🎮 Initializing R-Type Game...")

-- Configure Lua package path pour trouver les modules
package.path = package.path .. ";assets/scripts/?.lua;assets/scripts/?/init.lua"

-- Charger toutes les configurations
require("config.game_config")
require("config.player_config")
require("config.weapons_config")
require("config.enemies_config")
require("config.bosses_config")
require("config.assets_paths")

print("[LUA] ✅ All configurations loaded")
print("[LUA] 🚀 R-Type ready!")

return {
    version = "1.0.0",
    initialized = true
}
