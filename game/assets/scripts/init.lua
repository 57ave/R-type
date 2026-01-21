-- ============================================================================
-- INIT.LUA - Point d'entrée principal R-Type
-- ============================================================================

print("========================================")
print("🚀 R-TYPE - INITIALIZATION")
print("========================================")

-- Chemin de base
local basePath = ASSET_BASE_PATH or ""

-- Helper pour charger un script
local function LoadScript(relativePath, required)
    local fullPath = basePath .. "game/assets/scripts/" .. relativePath
    print("  Loading: " .. fullPath)
    
    local success, err = pcall(dofile, fullPath)
    if not success then
        if required then
            error("❌ CRITICAL: Cannot load " .. relativePath .. "\n" .. tostring(err))
        else
            print("  ⚠️  Warning: " .. relativePath .. " not found")
            return false
        end
    end
    
    print("  ✓ " .. relativePath .. " loaded")
    return true
end

-- Charger les configurations
print("\n--- Loading Configurations ---")
LoadScript("config/master_config.lua", false)
LoadScript("config/gameplay_config.lua", false)
LoadScript("config/enemies_config.lua", true)
LoadScript("config/bosses_config.lua", false)
LoadScript("config/weapons_config.lua", false)

print("\n✓ Configuration loaded successfully!")
print("========================================\n")

-- Variables globales du jeu
GameMode = {
    current = "solo",  -- "solo", "network"
    showcaseEnabled = false
}

-- Fonction appelée quand le jeu démarre en mode solo
function InitSoloMode()
    print("[GAME] Initialisation du mode SOLO")
    GameMode.current = "solo"
end

-- Fonction appelée quand le jeu démarre en mode réseau
function InitNetworkMode()
    print("[GAME] Initialisation du mode RÉSEAU")
    GameMode.current = "network"
end
