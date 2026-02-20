-- R-Type Game - Init Script
-- Loaded at startup, pulls in all config modules

package.path = package.path .. ";assets/scripts/?.lua;assets/scripts/?/init.lua"

require("config.game_config")
require("config.player_config")
require("config.weapons_config")
require("config.enemies_config")
require("config.bosses_config")
require("config.assets_paths")
require("config.patterns")

return {
    version = "1.0.0",
    initialized = true
}
