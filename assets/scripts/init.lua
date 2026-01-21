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
LoadConfig("assets/scripts/config/network_config.lua", true)     -- Network configuration
LoadConfig("assets/scripts/config/game_config.lua", false)       -- Game configuration  
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

-- Fonction appelée quand le jeu démarre en mode solo
function InitSoloMode()
    print("[GAME] Initialisation du mode SOLO")
    GameMode.current = "solo"
    
    -- En mode solo, activer le showcase automatiquement si demandé
    if AUTO_START_SHOWCASE then
        print("[GAME] AUTO_START_SHOWCASE activé - Lancement du showcase")
        if ToggleShowcaseMode then
            ToggleShowcaseMode()
            GameMode.showcaseEnabled = true
        end
    end
end

-- Fonction appelée quand le jeu démarre en mode réseau
function InitNetworkMode()
    print("[GAME] Initialisation du mode RÉSEAU")
    GameMode.current = "network"
    GameMode.showcaseEnabled = false
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

-- Active ou désactive le showcase automatique au démarrage en solo
AUTO_START_SHOWCASE = false  -- Mettre à false pour désactiver

print("Configuration:")
print("  • Auto-start showcase: " .. tostring(AUTO_START_SHOWCASE))
print("")

-- ============================================================================
-- VICTORY CALLBACK
-- ============================================================================

-- Function called when the player wins (survives 30 seconds)
function OnVictory()
    print("🎉 [VICTORY] Player has won the game!")
    
    -- Play victory sound if audio system is available
    if Audio and Audio.PlaySound then
        Audio.PlaySound("victory", 100)  -- Play victory sound at full volume
    end
    
    -- Could add particle effects, special music, etc. here
    print("🏆 Congratulations on surviving 30 seconds!")
end

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
