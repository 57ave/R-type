-- ============================================================================
-- UI_INIT.LUA - Menu Principal R-Type
-- ============================================================================

print("[UI] Loading ui_init.lua...")

-- Variables globales UI
local mainMenuEntities = {}
local currentMenu = "main"

-- ============================================================================
-- FONCTION PRINCIPALE D'INITIALISATION
-- ============================================================================
function InitUI(screenWidth, screenHeight)
    print("[UI] Initializing UI - " .. screenWidth .. "x" .. screenHeight)
    
    -- Définir l'état initial
    if GameState and GameState.Set then
        GameState.Set("MainMenu")
    end
    
    -- Créer le menu principal
    CreateMainMenu(screenWidth, screenHeight)
    
    print("[UI] UI initialized successfully")
end

-- ============================================================================
-- MENU PRINCIPAL
-- ============================================================================
function CreateMainMenu(screenWidth, screenHeight)
    local centerX = screenWidth / 2
    local centerY = screenHeight / 2
    
    print("[UI] Creating main menu at center: " .. centerX .. ", " .. centerY)
    
    -- Vérifier si UI est disponible
    if not UI then
        print("[UI] WARNING: UI functions not available, skipping menu creation")
        print("[UI] Game will start directly in Playing mode")
        -- Fallback : démarrer directement le jeu
        if GameState and GameState.Set then
            GameState.Set("Playing")
        end
        return
    end
    
    -- Titre
    if UI.CreateText then
        local title = UI.CreateText({
            text = "R-TYPE",
            x = centerX - 150,
            y = centerY - 200,
            fontSize = 72,
            color = {r = 255, g = 255, b = 255, a = 255},
            font = "default",
            menuGroup = "main"
        })
        table.insert(mainMenuEntities, title)
        print("[UI] Title created: entity " .. tostring(title))
    end
    
    -- Bouton PLAY
    if UI.CreateButton then
        local btnPlay = UI.CreateButton({
            text = "PLAY",
            x = centerX - 100,
            y = centerY - 50,
            width = 200,
            height = 50,
            onClick = "OnPlayClicked",
            menuGroup = "main"
        })
        table.insert(mainMenuEntities, btnPlay)
        print("[UI] Play button created: entity " .. tostring(btnPlay))
    end
    
    -- Bouton OPTIONS
    if UI.CreateButton then
        local btnOptions = UI.CreateButton({
            text = "OPTIONS",
            x = centerX - 100,
            y = centerY + 20,
            width = 200,
            height = 50,
            onClick = "OnOptionsClicked",
            menuGroup = "main"
        })
        table.insert(mainMenuEntities, btnOptions)
        print("[UI] Options button created: entity " .. tostring(btnOptions))
    end
    
    -- Bouton QUIT
    if UI.CreateButton then
        local btnQuit = UI.CreateButton({
            text = "QUIT",
            x = centerX - 100,
            y = centerY + 90,
            width = 200,
            height = 50,
            onClick = "OnQuitClicked",
            menuGroup = "main"
        })
        table.insert(mainMenuEntities, btnQuit)
        print("[UI] Quit button created: entity " .. tostring(btnQuit))
    end
    
    print("[UI] Main menu created with " .. #mainMenuEntities .. " elements")
end

-- ============================================================================
-- CALLBACKS DES BOUTONS
-- ============================================================================
function OnPlayClicked()
    print("[UI] Play clicked - Starting game")
    
    -- Cacher le menu
    if UI and UI.HideMenu then
        UI.HideMenu("main")
    end
    
    -- Démarrer le jeu
    if GameState and GameState.Set then
        GameState.Set("Playing")
    end
    
    -- Spawner le joueur
    if Game and Game.CreatePlayer then
        local playerX = 100
        local playerY = 540  -- Milieu de l'écran (1080 / 2)
        Game.CreatePlayer(playerX, playerY, 0)  -- line = 0
        print("[UI] Player spawned at (" .. playerX .. ", " .. playerY .. ")")
    else
        print("[UI] WARNING: Game.CreatePlayer not available")
    end
end

function OnOptionsClicked()
    print("[UI] Options clicked")
    if GameState and GameState.Set then
        GameState.Set("Options")
    end
    -- TODO: Créer le menu options
end

function OnQuitClicked()
    print("[UI] Quit clicked")
    -- TODO: Implémenter la fermeture propre
    os.exit(0)
end

print("[UI] ui_init.lua loaded successfully")
