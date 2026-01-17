-- ============================================================================
-- ENEMY & BOSS SHOWCASE MODE - VERSION LUA CONFIG
-- Utilise les configurations enemies_config.lua et bosses_config.lua
-- ============================================================================

EnemyShowcase = {
    active = false,
    spawnInterval = 1.5,  -- Secondes entre chaque spawn
    currentIndex = 1,
    lastSpawnTime = 0,
    
    -- Liste de tous les ennemis à afficher (depuis enemies_config.lua)
    enemyQueue = {
        -- BASIC ENEMIES
        { type = "basic", y = 200, info = "Basic - Patapata" },
        { type = "zigzag", y = 300, info = "Zigzag - Ziggy" },
        { type = "sinewave", y = 400, info = "Sinewave - Weaver" },
        { type = "kamikaze", y = 500, info = "Kamikaze - Crasher" },
        
        -- MEDIUM ENEMIES  
        { type = "shooter", y = 200, info = "Shooter - Gunner" },
        { type = "spreader", y = 300, info = "Spreader" },
        { type = "armored", y = 400, info = "Armored - Tank" },
        
        -- ELITE ENEMIES
        { type = "turret", y = 250, info = "Turret" },
        { type = "elite_fighter", y = 350, info = "Elite Fighter - Ace" },
        { type = "formation_leader", y = 450, info = "Formation Leader - Commander" },
        
        -- SPECIAL ENEMIES
        { type = "carrier", y = 300, info = "Carrier - Cargo" },
        { type = "shielded", y = 400, info = "Shielded - Barrier" },
        
        -- BOSSES (si disponibles)
        -- Les boss seront ajoutés dynamiquement si BossesConfig existe
    }
}

-- Fonction pour activer/désactiver le mode showcase
function ToggleShowcaseMode()
    EnemyShowcase.active = not EnemyShowcase.active
    
    if EnemyShowcase.active then
        print("[SHOWCASE] Mode de test des ennemis ACTIVÉ")
        print("[SHOWCASE] " .. #EnemyShowcase.enemyQueue .. " ennemis seront affichés")
        EnemyShowcase.currentIndex = 1
        EnemyShowcase.lastSpawnTime = 0
    else
        print("[SHOWCASE] Mode de test des ennemis DÉSACTIVÉ")
    end
    
    return EnemyShowcase.active
end

-- Fonction pour spawner un ennemi depuis les configs Lua
function SpawnEnemyFromConfig(enemyType, yPosition)
    -- Position de spawn (hors écran à droite)
    local spawnX = 1920 + 100
    local spawnY = yPosition or 540
    
    print("[SHOWCASE] Spawning Enemy: " .. enemyType .. " at Y=" .. spawnY)
    
    -- Vérifier que Factory existe
    if not Factory then
        print("[SHOWCASE] ✗ ERROR: Factory not found!")
        return nil
    end
    
    if not Factory.CreateEnemyFromConfig then
        print("[SHOWCASE] ✗ ERROR: Factory.CreateEnemyFromConfig not found!")
        print("[SHOWCASE]    Did you add the new C++ code?")
        return nil
    end
    
    -- Vérifier que la config existe
    if not EnemiesConfig then
        print("[SHOWCASE] ✗ ERROR: EnemiesConfig not loaded!")
        return nil
    end
    
    if not EnemiesConfig[enemyType] then
        print("[SHOWCASE] ✗ ERROR: Enemy type '" .. enemyType .. "' not found in EnemiesConfig!")
        print("[SHOWCASE]    Available types:")
        for key, _ in pairs(EnemiesConfig) do
            print("[SHOWCASE]      - " .. key)
        end
        return nil
    end
    
    -- Créer l'ennemi via la Factory avec la config Lua
    local entity = Factory.CreateEnemyFromConfig(enemyType, spawnX, spawnY)
    
    if entity and entity ~= 0 then
        local config = EnemiesConfig[enemyType]
        local name = config.name or enemyType
        print("[SHOWCASE] ✓ " .. name .. " spawned successfully! Entity ID: " .. entity)
        
        -- Afficher quelques infos
        local health = config.health or "?"
        local speed = config.speed or "?"
        local pattern = (config.movement and config.movement.pattern) or "?"
        print("[SHOWCASE]   - Health: " .. health .. " HP")
        print("[SHOWCASE]   - Speed: " .. speed)
        print("[SHOWCASE]   - Pattern: " .. pattern)
    else
        print("[SHOWCASE] ✗ Failed to spawn " .. enemyType)
    end
    
    return entity
end

-- Fonction pour mettre à jour le showcase (appelée chaque frame)
function UpdateShowcase(deltaTime)
    if not EnemyShowcase.active then
        return
    end
    
    EnemyShowcase.lastSpawnTime = EnemyShowcase.lastSpawnTime + deltaTime
    
    -- Temps de spawner le prochain ennemi ?
    if EnemyShowcase.lastSpawnTime >= EnemyShowcase.spawnInterval then
        EnemyShowcase.lastSpawnTime = 0
        
        -- Y a-t-il encore des ennemis à spawner ?
        if EnemyShowcase.currentIndex <= #EnemyShowcase.enemyQueue then
            local enemyData = EnemyShowcase.enemyQueue[EnemyShowcase.currentIndex]
            
            print("")
            print("========================================")
            print("[SHOWCASE] Test " .. EnemyShowcase.currentIndex .. "/" .. #EnemyShowcase.enemyQueue)
            print("[SHOWCASE] " .. enemyData.info)
            print("========================================")
            
            SpawnEnemyFromConfig(enemyData.type, enemyData.y)
            
            EnemyShowcase.currentIndex = EnemyShowcase.currentIndex + 1
        else
            -- Tous les ennemis ont été affichés
            print("")
            print("========================================")
            print("[SHOWCASE] Tous les ennemis ont été testés!")
            print("[SHOWCASE] Désactivation du mode showcase...")
            print("========================================")
            EnemyShowcase.active = false
        end
    end
end

-- Fonction pour afficher tous les ennemis en grille
function ShowAllEnemiesGrid()
    print("[SHOWCASE] Affichage de tous les ennemis disponibles en grille...")
    
    if not EnemiesConfig then
        print("[SHOWCASE] ✗ EnemiesConfig not loaded!")
        return
    end
    
    local startX = 400
    local startY = 100
    local spacingY = 120
    local currentY = startY
    local count = 0
    
    for enemyType, config in pairs(EnemiesConfig) do
        if type(config) == "table" then
            SpawnEnemyFromConfig(enemyType, currentY)
            currentY = currentY + spacingY
            count = count + 1
            
            -- Limiter pour ne pas dépasser l'écran
            if currentY > 900 then
                break
            end
        end
    end
    
    print("[SHOWCASE] " .. count .. " ennemis affichés en grille!")
end

-- Fonction pour tester un ennemi spécifique
function TestSpecificEnemy(enemyType, yPosition)
    yPosition = yPosition or 540
    print("[SHOWCASE] Test manuel de: " .. enemyType)
    SpawnEnemyFromConfig(enemyType, yPosition)
end

-- Fonction pour lister tous les ennemis disponibles
function ListAllEnemies()
    print("")
    print("========================================")
    print("ENNEMIS DISPONIBLES (depuis Lua configs):")
    print("========================================")
    
    if not EnemiesConfig then
        print("✗ EnemiesConfig not loaded!")
        return
    end
    
    -- Grouper par catégorie
    local categories = {
        common = {},
        medium = {},
        elite = {},
        special = {}
    }
    
    for enemyType, config in pairs(EnemiesConfig) do
        if type(config) == "table" then
            local category = config.category or "common"
            if not categories[category] then
                categories[category] = {}
            end
            table.insert(categories[category], {
                type = enemyType,
                name = config.name or enemyType,
                health = config.health or "?",
                score = config.scoreValue or "?"
            })
        end
    end
    
    -- Afficher par catégorie
    for category, enemies in pairs(categories) do
        if #enemies > 0 then
            print("\n--- " .. category:upper() .. " ---")
            for _, enemy in ipairs(enemies) do
                print(string.format("  • %-15s (%s) - HP: %d, Score: %d", 
                    enemy.type, enemy.name, enemy.health, enemy.score))
            end
        end
    end
    
    print("\n========================================")
    print("Utilisez: SpawnEnemy('type') pour spawner")
    print("========================================")
end

-- ============================================================================
-- COMMANDES DE TEST RAPIDES
-- ============================================================================

-- Activer le showcase automatique
function StartShowcase()
    ToggleShowcaseMode()
end

-- Afficher un ennemi spécifique
function SpawnEnemy(enemyType)
    TestSpecificEnemy(enemyType)
end

-- Afficher tous les ennemis basic
function ShowBasicEnemies()
    TestSpecificEnemy("basic", 200)
    TestSpecificEnemy("zigzag", 350)
    TestSpecificEnemy("sinewave", 500)
    TestSpecificEnemy("kamikaze", 650)
end

-- Afficher tous les ennemis medium
function ShowMediumEnemies()
    TestSpecificEnemy("shooter", 250)
    TestSpecificEnemy("spreader", 400)
    TestSpecificEnemy("armored", 550)
end

-- Afficher tous les ennemis elite
function ShowEliteEnemies()
    TestSpecificEnemy("turret", 200)
    TestSpecificEnemy("elite_fighter", 400)
    TestSpecificEnemy("formation_leader", 600)
end

-- Afficher tous les ennemis special
function ShowSpecialEnemies()
    TestSpecificEnemy("carrier", 300)
    TestSpecificEnemy("shielded", 500)
end

-- Spawner plusieurs ennemis du même type
function SpawnWave(enemyType, count, spacing)
    spacing = spacing or 100
    for i = 1, count do
        local y = 200 + ((i - 1) * spacing)
        TestSpecificEnemy(enemyType, y)
    end
end

-- ============================================================================
-- INITIALISATION
-- ============================================================================

print("")
print("========================================")
print("ENEMY SHOWCASE MODE - LUA CONFIG VERSION")
print("========================================")
print("Utilise les configurations depuis enemies_config.lua")
print("")
print("Commandes disponibles:")
print("  • StartShowcase()          - Lance le showcase automatique")
print("  • SpawnEnemy('type')       - Spawn un ennemi spécifique")
print("  • ShowBasicEnemies()       - Affiche tous les ennemis basic")
print("  • ShowMediumEnemies()      - Affiche tous les ennemis medium")
print("  • ShowEliteEnemies()       - Affiche tous les ennemis elite")
print("  • ShowSpecialEnemies()     - Affiche tous les ennemis special")
print("  • ListAllEnemies()         - Liste tous les ennemis disponibles")
print("  • ShowAllEnemiesGrid()     - Affiche tous en grille")
print("  • SpawnWave('type', n)     - Spawn n ennemis du même type")
print("========================================")
print("")

-- Vérifier que les configs sont chargées
if EnemiesConfig then
    print("[SHOWCASE] ✓ EnemiesConfig loaded successfully")
    local count = 0
    for _ in pairs(EnemiesConfig) do count = count + 1 end
    print("[SHOWCASE]   Found " .. count .. " enemy types")
else
    print("[SHOWCASE] ✗ WARNING: EnemiesConfig not loaded!")
end

if Factory and Factory.ListEnemyTypes then
    print("[SHOWCASE] ✓ Factory.ListEnemyTypes available")
    print("[SHOWCASE]   Use Factory.ListEnemyTypes() to see all types")
else
    print("[SHOWCASE] ⚠ Factory.ListEnemyTypes not available")
end

print("")