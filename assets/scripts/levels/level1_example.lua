-- ============================================================================
-- LEVEL 1 EXAMPLE - "First Contact"
-- Exemple de niveau personnalisé en Lua
-- ============================================================================

Level1 = {
    name = "First Contact",
    description = "Premier contact avec l'ennemi",
    difficulty = "easy",
    duration = 180,  -- 3 minutes
    
    -- Configuration du background
    background = {
        texture = "background.png",
        scrollSpeed = 200
    },
    
    -- Musique de fond
    music = "stage1_theme.ogg",
    
    -- Vagues d'ennemis
    waves = {
        -- Vague 1: Introduction (0-10 secondes)
        {
            time = 0,
            name = "Tutorial Wave",
            enemies = {
                { type = "basic", x = 1920, y = 200, delay = 0 },
                { type = "basic", x = 1920, y = 400, delay = 1 },
                { type = "basic", x = 1920, y = 600, delay = 2 }
            }
        },
        
        -- Vague 2: Formation en ligne (10-20 secondes)
        {
            time = 10,
            name = "Line Formation",
            enemies = {
                { type = "basic", x = 1920, y = 150, delay = 0 },
                { type = "basic", x = 1920, y = 250, delay = 0.2 },
                { type = "basic", x = 1920, y = 350, delay = 0.4 },
                { type = "basic", x = 1920, y = 450, delay = 0.6 },
                { type = "basic", x = 1920, y = 550, delay = 0.8 }
            }
        },
        
        -- Vague 3: Introduction des zigzag (20-30 secondes)
        {
            time = 20,
            name = "Zigzag Introduction",
            enemies = {
                { type = "zigzag", x = 1920, y = 200, delay = 0 },
                { type = "zigzag", x = 1920, y = 540, delay = 1 },
                { type = "basic", x = 1920, y = 880, delay = 2 }
            }
        },
        
        -- Vague 4: Mélange (30-40 secondes)
        {
            time = 30,
            name = "Mixed Formation",
            enemies = {
                { type = "sinewave", x = 1920, y = 300, delay = 0 },
                { type = "zigzag", x = 1920, y = 540, delay = 0.5 },
                { type = "basic", x = 1920, y = 200, delay = 1 },
                { type = "basic", x = 1920, y = 880, delay = 1.5 }
            }
        },
        
        -- Vague 5: Kamikaze rush (40-50 secondes)
        {
            time = 40,
            name = "Kamikaze Rush",
            enemies = {
                { type = "kamikaze", x = 1920, y = 200, delay = 0 },
                { type = "kamikaze", x = 1920, y = 400, delay = 0.5 },
                { type = "kamikaze", x = 1920, y = 600, delay = 1 },
                { type = "kamikaze", x = 1920, y = 800, delay = 1.5 }
            }
        },
        
        -- Vague 6: Mini-boss (60-80 secondes)
        {
            time = 60,
            name = "Mid-Boss Formation Leader",
            enemies = {
                { type = "formation_leader", x = 1920, y = 540, delay = 0 },
                { type = "basic", x = 1920, y = 200, delay = 2 },
                { type = "basic", x = 1920, y = 880, delay = 2 }
            }
        },
        
        -- Vague 7: Tireurs (90-100 secondes)
        {
            time = 90,
            name = "Shooters Introduction",
            enemies = {
                { type = "shooter", x = 1920, y = 300, delay = 0 },
                { type = "shooter", x = 1920, y = 780, delay = 1 },
                { type = "basic", x = 1920, y = 540, delay = 2 }
            }
        },
        
        -- Vague 8: Dernière vague avant le boss (150-160 secondes)
        {
            time = 150,
            name = "Final Wave",
            enemies = {
                { type = "elite_fighter", x = 1920, y = 300, delay = 0 },
                { type = "spreader", x = 1920, y = 540, delay = 1 },
                { type = "elite_fighter", x = 1920, y = 780, delay = 2 },
                { type = "shooter", x = 1920, y = 200, delay = 3 },
                { type = "shooter", x = 1920, y = 880, delay = 3 }
            }
        },
        
        -- Boss Final (170 secondes)
        {
            time = 170,
            name = "Boss Battle",
            boss = {
                type = "stage1_boss",
                x = 1920,
                y = 540
            },
            -- Support pendant le boss
            supportWaves = {
                {
                    time = 10,  -- 10 secondes après le début du boss
                    enemies = {
                        { type = "basic", x = 1920, y = 200 },
                        { type = "basic", x = 1920, y = 880 }
                    }
                },
                {
                    time = 30,
                    enemies = {
                        { type = "kamikaze", x = 1920, y = 300 },
                        { type = "kamikaze", x = 1920, y = 780 }
                    }
                }
            }
        }
    },
    
    -- Power-ups garantis à certains moments
    powerups = {
        { time = 25, type = "weapon_upgrade", x = 1920, y = 540 },
        { time = 55, type = "health_restore", x = 1920, y = 300 },
        { time = 95, type = "speed_boost", x = 1920, y = 540 },
        { time = 145, type = "shield", x = 1920, y = 540 }
    },
    
    -- Objectifs secondaires
    objectives = {
        {
            name = "No Deaths",
            description = "Complete without dying",
            reward = 5000
        },
        {
            name = "Speed Run",
            description = "Complete in under 2:30",
            reward = 3000
        },
        {
            name = "Perfect",
            description = "Destroy all enemies",
            reward = 10000
        }
    }
}

-- ============================================================================
-- FONCTIONS DE GESTION DU NIVEAU
-- ============================================================================

-- État du niveau
local levelState = {
    currentTime = 0,
    waveIndex = 1,
    isActive = false,
    bossSpawned = false,
    enemiesSpawned = {},
    powerupsSpawned = {}
}

-- Démarrer le niveau
function StartLevel1()
    print("")
    print("========================================")
    print("🎮 DÉMARRAGE DU NIVEAU 1")
    print("========================================")
    print("Nom: " .. Level1.name)
    print("Description: " .. Level1.description)
    print("Durée: " .. Level1.duration .. " secondes")
    print("Vagues: " .. #Level1.waves)
    print("========================================")
    print("")
    
    levelState.currentTime = 0
    levelState.waveIndex = 1
    levelState.isActive = true
    levelState.bossSpawned = false
    levelState.enemiesSpawned = {}
    levelState.powerupsSpawned = {}
    
    return true
end

-- Mettre à jour le niveau (appelé chaque frame)
function UpdateLevel1(deltaTime)
    if not levelState.isActive then
        return
    end
    
    levelState.currentTime = levelState.currentTime + deltaTime
    
    -- Vérifier si on doit spawner une nouvelle vague
    if levelState.waveIndex <= #Level1.waves then
        local wave = Level1.waves[levelState.waveIndex]
        
        if levelState.currentTime >= wave.time then
            SpawnWave(wave, levelState.waveIndex)
            levelState.waveIndex = levelState.waveIndex + 1
        end
    end
    
    -- Vérifier les power-ups
    for i, powerup in ipairs(Level1.powerups) do
        if not levelState.powerupsSpawned[i] and levelState.currentTime >= powerup.time then
            SpawnPowerup(powerup)
            levelState.powerupsSpawned[i] = true
        end
    end
    
    -- Vérifier la fin du niveau
    if levelState.currentTime >= Level1.duration then
        EndLevel1()
    end
end

-- Spawner une vague d'ennemis
function SpawnWave(wave, waveNum)
    print("")
    print("========================================")
    print("🌊 VAGUE " .. waveNum .. ": " .. wave.name)
    print("========================================")
    
    -- Spawner le boss si c'est une vague de boss
    if wave.boss then
        local boss = wave.boss
        print("🦴 BOSS INCOMING: " .. boss.type)
        
        if SpawnTestEnemy then
            SpawnTestEnemy(boss.type, boss.y or 540)
        end
        
        levelState.bossSpawned = true
        
        -- TODO: Gérer les vagues de support pendant le boss
        return
    end
    
    -- Spawner les ennemis normaux
    if wave.enemies then
        for i, enemy in ipairs(wave.enemies) do
            local delay = enemy.delay or 0
            
            -- TODO: Implémenter le système de délai
            -- Pour l'instant, spawn immédiat
            if SpawnTestEnemy then
                print("  Spawning " .. enemy.type .. " at Y=" .. enemy.y)
                SpawnTestEnemy(enemy.type, enemy.y)
            end
        end
    end
    
    print("✓ Vague spawned")
end

-- Spawner un power-up
function SpawnPowerup(powerup)
    print("💎 Power-up spawned: " .. powerup.type .. " at Y=" .. powerup.y)
    -- TODO: Implémenter le spawn de power-up
end

-- Terminer le niveau
function EndLevel1()
    print("")
    print("========================================")
    print("🏁 NIVEAU 1 TERMINÉ!")
    print("========================================")
    print("Temps total: " .. string.format("%.1f", levelState.currentTime) .. "s")
    print("========================================")
    print("")
    
    levelState.isActive = false
    
    -- TODO: Calculer le score, vérifier les objectifs, etc.
end

-- Réinitialiser le niveau
function ResetLevel1()
    levelState = {
        currentTime = 0,
        waveIndex = 1,
        isActive = false,
        bossSpawned = false,
        enemiesSpawned = {},
        powerupsSpawned = {}
    }
    
    print("[Level1] Reset")
end

-- ============================================================================
-- COMMANDES DISPONIBLES
-- ============================================================================

print("")
print("📋 Commandes du Niveau 1:")
print("  • StartLevel1()     - Démarrer le niveau")
print("  • ResetLevel1()     - Réinitialiser le niveau")
print("")
