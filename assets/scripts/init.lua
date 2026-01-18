-- ============================================================================
-- INIT.LUA - Configuration principale du jeu
-- Charge tous les fichiers de configuration et initialise les systèmes
-- ============================================================================

print("")
print("========================================")
print("🚀 INITIALISATION R-TYPE")
print("========================================")

-- ============================================================================
-- CHARGEMENT DES CONFIGURATIONS
-- ============================================================================

print("📦 Chargement des fichiers de configuration...")

-- Déterminer le chemin de base des assets
local assetBasePath = ASSET_BASE_PATH or ""

-- Fonction helper pour charger un script
local function LoadConfig(filename, required)
    local fullPath = assetBasePath .. filename
    print("  Loading: " .. fullPath)
    
    local success, err = pcall(dofile, fullPath)
    if not success then
        if required then
            error("❌ ERREUR CRITIQUE: Impossible de charger " .. filename .. "\n" .. tostring(err))
        else
            print("  ⚠️ Warning: " .. filename .. " not found or error loading")
            return false
        end
    end
    
    print("  ✓ " .. filename .. " loaded successfully")
    return true
end

-- Charger tous les fichiers de configuration
print("\n--- Configuration Files ---")
LoadConfig("assets/scripts/master_config.lua", false)
LoadConfig("assets/scripts/gameplay_config.lua", false)
LoadConfig("assets/scripts/enemies_config.lua", true)   -- Required
LoadConfig("assets/scripts/bosses_config.lua", true)    -- Required
LoadConfig("assets/scripts/weapons_config.lua", false)
LoadConfig("assets/scripts/powerups_config.lua", false)
LoadConfig("assets/scripts/stages_config.lua", false)

-- Charger le système de showcase des ennemis
print("\n--- Enemy Showcase System ---")
LoadConfig("assets/scripts/enemy_showcase.lua", false)

print("")
print("========================================")
print("✓ Configuration chargée avec succès!")
print("========================================")
print("")

-- ============================================================================
-- VARIABLES GLOBALES DU JEU
-- ============================================================================

-- Mode de jeu
GameMode = {
    current = "solo",  -- "solo", "network"
    showcaseEnabled = false
}

-- ============================================================================
-- FONCTIONS D'INITIALISATION DU GAMEPLAY
-- ============================================================================

-- Fonction appelée quand le jeu démarre en mode solo (DEPRECATED - redirige vers network)
function InitSoloMode()
    print("[GAME] Solo mode is DEPRECATED - redirecting to network mode")
    InitNetworkMode()  -- Redirige vers le mode réseau
end

-- Fonction appelée quand le jeu démarre en mode réseau (toujours le cas maintenant)
function InitNetworkMode()
    print("[GAME] Initialisation du mode RÉSEAU (mode par défaut)")
    GameMode.current = "network"
    GameMode.showcaseEnabled = false
    
    -- Note: Le showcase est désactivé en mode réseau
    print("[GAME] Showcase désactivé (mode réseau actif)")
end

-- Fonction de mise à jour appelée chaque frame
function UpdateGame(deltaTime)
    -- Si le showcase est actif, le mettre à jour
    if GameMode.showcaseEnabled and UpdateShowcase then
        UpdateShowcase(deltaTime)
    end
end

-- ============================================================================
-- CONFIGURATION PAR DÉFAUT
-- ============================================================================

-- Le showcase automatique est toujours désactivé en mode réseau
AUTO_START_SHOWCASE = false

print("Configuration:")
print("  • Mode: NETWORK ONLY (par défaut)")
print("  • Auto-start showcase: DISABLED (mode réseau)")
print("")

-- ============================================================================
-- COMMANDES DISPONIBLES
-- ============================================================================

print("📋 Commandes disponibles:")
print("  • InitSoloMode()           - Initialise le mode solo")
print("  • InitNetworkMode()        - Initialise le mode réseau")
print("  • StartShowcase()          - Lance le showcase d'ennemis")
print("  • SpawnEnemy('type')       - Spawn un ennemi spécifique")
print("  • ListAllEnemies()         - Liste tous les ennemis")
print("")
