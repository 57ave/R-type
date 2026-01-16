-- ============================================================================
-- ENEMY & BOSS SHOWCASE MODE
-- Ce fichier permet de tester visuellement tous les ennemis et boss
-- ============================================================================

EnemyShowcase = {
    active = false,
    spawnInterval = 1.5,  -- Secondes entre chaque spawn
    currentIndex = 1,
    lastSpawnTime = 0,
    
    -- Liste de tous les ennemis à afficher dans l'ordre
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
        
        -- BOSSES
        { type = "stage1_boss", y = 540, info = "BOSS - Stage 1 Dobkeratops" },
        { type = "stage2_boss", y = 540, info = "BOSS - Stage 2 Gomander" },
        { type = "stage3_boss", y = 540, info = "BOSS - Stage 3 Big Core" },
    }
}

-- Fonction pour activer/désactiver le mode showcase
function ToggleShowcaseMode()
    EnemyShowcase.active = not EnemyShowcase.active
    
    if EnemyShowcase.active then
        print("[SHOWCASE] Mode de test des ennemis ACTIVÉ")
        print("[SHOWCASE] " .. #EnemyShowcase.enemyQueue .. " ennemis et boss seront affichés")
        EnemyShowcase.currentIndex = 1
        EnemyShowcase.lastSpawnTime = 0
    else
        print("[SHOWCASE] Mode de test des ennemis DÉSACTIVÉ")
    end
    
    return EnemyShowcase.active
end

-- Fonction pour spawner un ennemi de test
function SpawnTestEnemy(enemyType, yPosition)
    local enemyConfig = nil
    local isBoss = false
    
    -- Chercher dans les configs d'ennemis
    if EnemiesConfig and EnemiesConfig[enemyType] then
        enemyConfig = EnemiesConfig[enemyType]
    -- Chercher dans les configs de boss
    elseif BossesConfig and BossesConfig[enemyType] then
        enemyConfig = BossesConfig[enemyType]
        isBoss = true
    end
    
    if not enemyConfig then
        print("[SHOWCASE] Erreur: Ennemi/Boss '" .. enemyType .. "' non trouvé dans la config!")
        return nil
    end
    
    -- Position de spawn (hors écran à droite)
    local spawnX = 1920 + 100  -- Hors écran
    local spawnY = yPosition or 540
    
    -- Créer l'entité via la factory appropriée
    local entity = nil
    
    if isBoss then
        print("[SHOWCASE] Spawning BOSS: " .. enemyConfig.name .. " at Y=" .. spawnY)
        
        -- Pour les boss, utiliser BossFactory si disponible
        if BossFactory then
            entity = BossFactory.CreateBoss(enemyType, spawnX, spawnY)
        else
            print("[SHOWCASE] BossFactory non disponible, utilisation EnemyFactory")
            if EnemyFactory then
                entity = EnemyFactory.CreateEnemy(enemyType, spawnX, spawnY)
            end
        end
    else
        print("[SHOWCASE] Spawning Enemy: " .. enemyConfig.name .. " at Y=" .. spawnY)
        
        -- Pour les ennemis normaux
        if EnemyFactory then
            entity = EnemyFactory.CreateEnemy(enemyType, spawnX, spawnY)
        end
    end
    
    if entity then
        print("[SHOWCASE] ✓ " .. enemyConfig.name .. " spawned successfully!")
        print("[SHOWCASE]   - Texture: " .. enemyConfig.sprite.texture)
        print("[SHOWCASE]   - Frame size: " .. enemyConfig.sprite.frameWidth .. "x" .. enemyConfig.sprite.frameHeight)
        print("[SHOWCASE]   - Scale: " .. enemyConfig.sprite.scale)
        print("[SHOWCASE]   - Animation frames: " .. enemyConfig.animation.frameCount)
    else
        print("[SHOWCASE] ✗ Failed to spawn " .. enemyConfig.name)
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
            
            SpawnTestEnemy(enemyData.type, enemyData.y)
            
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

-- Fonction pour afficher tous les ennemis en même temps (mode grille)
function ShowAllEnemiesGrid()
    print("[SHOWCASE] Affichage de tous les ennemis en grille...")
    
    local startX = 400
    local startY = 100
    local spacingX = 200
    local spacingY = 150
    local perRow = 4
    
    for i, enemyData in ipairs(EnemyShowcase.enemyQueue) do
        local row = math.floor((i - 1) / perRow)
        local col = (i - 1) % perRow
        
        local x = startX + (col * spacingX)
        local y = startY + (row * spacingY)
        
        SpawnTestEnemy(enemyData.type, y)
    end
    
    print("[SHOWCASE] Grille complète affichée!")
end

-- Fonction pour tester un ennemi spécifique
function TestSpecificEnemy(enemyType, yPosition)
    yPosition = yPosition or 540
    print("[SHOWCASE] Test manuel de: " .. enemyType)
    SpawnTestEnemy(enemyType, yPosition)
end

-- Fonction pour lister tous les ennemis disponibles
function ListAllEnemies()
    print("")
    print("========================================")
    print("ENNEMIS DISPONIBLES:")
    print("========================================")
    
    if EnemiesConfig then
        print("\n--- ENNEMIS NORMAUX ---")
        for enemyType, config in pairs(EnemiesConfig) do
            print("  • " .. enemyType .. " : " .. config.name .. " (" .. config.category .. ")")
        end
    end
    
    if BossesConfig then
        print("\n--- BOSS ---")
        for bossType, config in pairs(BossesConfig) do
            print("  • " .. bossType .. " : " .. config.name)
        end
    end
    
    print("\n========================================")
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

-- Afficher tous les boss
function ShowAllBosses()
    if BossesConfig then
        for bossType, _ in pairs(BossesConfig) do
            TestSpecificEnemy(bossType, 540)
        end
    end
end

-- ============================================================================
-- INITIALISATION
-- ============================================================================

print("")
print("========================================")
print("ENEMY SHOWCASE MODE LOADED")
print("========================================")
print("Commandes disponibles:")
print("  • StartShowcase()          - Lance le showcase automatique")
print("  • SpawnEnemy('type')       - Spawn un ennemi spécifique")
print("  • ShowBasicEnemies()       - Affiche tous les ennemis basic")
print("  • ShowMediumEnemies()      - Affiche tous les ennemis medium")
print("  • ShowAllBosses()          - Affiche tous les boss")
print("  • ListAllEnemies()         - Liste tous les ennemis disponibles")
print("  • ShowAllEnemiesGrid()     - Affiche tous en grille")
print("========================================")
print("")
