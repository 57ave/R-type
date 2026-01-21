-- ============================================
-- R-TYPE UI INITIALIZATION SCRIPT
-- ============================================

-- ============================================
-- GLOBAL VARIABLES
-- ============================================

-- UI Element storage by menu group
local mainMenuElements = {}
local serverBrowserElements = {}
local createRoomElements = {}
local lobbyElements = {}
local settingsElements = {}
local pauseElements = {}

-- Lobby data (will be synchronized with server later)
local lobbyData = {
    rooms = {},           -- Available rooms list
    currentRoom = nil,    -- Current room if in lobby
    players = {},         -- Players in current room
    isHost = false,
    isReady = false,
    selectedServerId = nil,
    joiningRoom = false   -- NOUVEAU: Flag pour éviter les double-clics
}

-- Settings data
local settingsData = {
    musicVolume = 70,
    sfxVolume = 80,
    resolution = 1,       -- Index: 1 = 1920x1080, 2 = 1280x720, 3 = 1600x900
    fullscreen = false
}

-- Animation state
local animations = {
    titlePulse = { timer = 0, scale = 1.0, direction = 1, speed = 0.5 }
}

-- Screen dimensions (set in InitUI)
local screenWidth = 1920
local screenHeight = 1080

-- ============================================
-- COLOR CONSTANTS (R-Type Theme)
-- ============================================
local COLORS = {
    PRIMARY_BLUE = 0x1E90FFFF,      -- DodgerBlue
    DARK_BLUE = 0x0D1B2AFF,         -- Dark background
    ACCENT_ORANGE = 0xFF6600FF,     -- Hover/selection
    WHITE = 0xFFFFFFFF,
    GRAY = 0x808080FF,
    LIGHT_GRAY = 0xAAAAAAFF,
    PANEL_BG = 0x0D1B2ACC,          -- Semi-transparent
    BUTTON_BG = 0x1E90FF99,         -- Semi-transparent blue
    SUCCESS_GREEN = 0x00FF00FF,
    ERROR_RED = 0xFF0000FF
}

-- ============================================
-- BUTTON DIMENSIONS
-- ============================================
local BUTTON = {
    WIDTH = 300,
    HEIGHT = 60,
    SPACING = 20
}

-- ============================================
-- MAIN MENU CREATION
-- ============================================
function CreateMainMenu(centerX, centerY, width, height)
    local menuGroup = "main_menu"
    
    -- Title "R-TYPE" with large font
    mainMenuElements.title = UI.CreateText({
        x = centerX,
        y = 150,
        text = "R-TYPE",
        fontSize = 100,
        color = COLORS.PRIMARY_BLUE,
        menuGroup = menuGroup
    })
    
    -- Subtitle
    mainMenuElements.subtitle = UI.CreateText({
        x = centerX,
        y = 250,
        text = "ARCADE SHOOTER",
        fontSize = 24,
        color = COLORS.LIGHT_GRAY,
        menuGroup = menuGroup
    })
    
    -- Calculate button positions (centered vertically)
    local startY = centerY - 50
    local buttonX = centerX
    
    -- PLAY Button
    mainMenuElements.playBtn = UI.CreateButton({
        x = buttonX,
        y = startY,
        width = BUTTON.WIDTH,
        height = BUTTON.HEIGHT,
        text = "PLAY",
        onClick = "OnPlayClicked",
        menuGroup = menuGroup
    })
    
    -- MULTIPLAYER Button
    mainMenuElements.multiplayerBtn = UI.CreateButton({
        x = buttonX,
        y = startY + BUTTON.HEIGHT + BUTTON.SPACING,
        width = BUTTON.WIDTH,
        height = BUTTON.HEIGHT,
        text = "MULTIPLAYER",
        onClick = "OnMultiplayerClicked",
        menuGroup = menuGroup
    })
    
    -- SETTINGS Button
    mainMenuElements.settingsBtn = UI.CreateButton({
        x = buttonX,
        y = startY + (BUTTON.HEIGHT + BUTTON.SPACING) * 2,
        width = BUTTON.WIDTH,
        height = BUTTON.HEIGHT,
        text = "SETTINGS",
        onClick = "OnSettingsClicked",
        menuGroup = menuGroup
    })
    
    -- QUIT Button
    mainMenuElements.quitBtn = UI.CreateButton({
        x = buttonX,
        y = startY + (BUTTON.HEIGHT + BUTTON.SPACING) * 3,
        width = BUTTON.WIDTH,
        height = BUTTON.HEIGHT,
        text = "QUIT",
        onClick = "OnQuitClicked",
        menuGroup = menuGroup
    })
    
    -- Version text at bottom
    mainMenuElements.version = UI.CreateText({
        x = centerX,
        y = height - 50,
        text = "v1.0.0 - ECS Engine",
        fontSize = 16,
        color = COLORS.GRAY,
        menuGroup = menuGroup
    })
end

-- ============================================
-- SERVER BROWSER MENU
-- ============================================
function CreateServerBrowser(centerX, centerY, width, height)
    local menuGroup = "server_browser"
    
    -- Title
    serverBrowserElements.title = UI.CreateText({
        x = centerX,
        y = 80,
        text = "MULTIPLAYER",
        fontSize = 60,
        color = COLORS.PRIMARY_BLUE,
        menuGroup = menuGroup
    })
    
    -- Server list panel
    serverBrowserElements.serverPanel = UI.CreatePanel({
        x = centerX - 350,
        y = 150,
        width = 700,
        height = 400,
        backgroundColor = COLORS.PANEL_BG,
        modal = false,
        menuGroup = menuGroup
    })
    
    -- Server list header (RENOMMÉ: SERVERS → ROOMS)
    serverBrowserElements.serverHeader = UI.CreateText({
        x = centerX,
        y = 170,
        text = "AVAILABLE ROOMS",
        fontSize = 24,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    -- Instructions pour l'utilisateur
    serverBrowserElements.instructionsText = UI.CreateText({
        x = centerX,
        y = 210,
        text = "Click on a room to join, or create your own",
        fontSize = 16,
        color = COLORS.LIGHT_GRAY,
        menuGroup = menuGroup
    })
    
    -- Placeholder for server list (will be populated dynamically)
    serverBrowserElements.noServersText = UI.CreateText({
        x = centerX,
        y = 350,
        text = "No rooms available. Click REFRESH to update or CREATE ROOM to start one.",
        fontSize = 18,
        color = COLORS.GRAY,
        menuGroup = menuGroup
    })
    
    -- Server entries (created dynamically, store references)
    serverBrowserElements.serverEntries = {}
    
    -- Manual IP input section
    serverBrowserElements.ipLabel = UI.CreateText({
        x = centerX - 250,
        y = 580,
        text = "Direct Connect:",
        fontSize = 20,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    serverBrowserElements.ipInput = UI.CreateInputField({
        x = centerX - 100,
        y = 570,
        width = 250,
        height = 40,
        placeholder = "IP:PORT (e.g. 127.0.0.1:12345)",
        menuGroup = menuGroup
    })

    serverBrowserElements.connectedText = UI.CreateText({
        x = centerX,
        y = 640,
        text = "Not connected",
        fontSize = 16,
        color = COLORS.LIGHT_GRAY,
        menuGroup = menuGroup
    })
    
    serverBrowserElements.joinIpBtn = UI.CreateButton({
        x = centerX + 200,
        y = 590,
        width = 100,
        height = 40,
        text = "JOIN",
        onClick = "OnJoinByIP",
        menuGroup = menuGroup
    })
    
    -- Bottom buttons
    local bottomY = height - 120
    
    serverBrowserElements.refreshBtn = UI.CreateButton({
        x = centerX - 220,
        y = bottomY,
        width = 180,
        height = 50,
        text = "REFRESH",
        onClick = "OnRefreshServers",
        menuGroup = menuGroup
    })
    
    serverBrowserElements.createRoomBtn = UI.CreateButton({
        x = centerX,
        y = bottomY,
        width = 180,
        height = 50,
        text = "CREATE ROOM",
        onClick = "OnCreateRoomClicked",
        menuGroup = menuGroup
    })
    
    serverBrowserElements.backBtn = UI.CreateButton({
        x = centerX + 220,
        y = bottomY,
        width = 180,
        height = 50,
        text = "BACK",
        onClick = "OnBackToMainMenu",
        menuGroup = menuGroup
    })
end

-- ============================================
-- CREATE ROOM MENU
-- ============================================
function CreateRoomCreation(centerX, centerY, width, height)
    local menuGroup = "create_room_menu"
    
    -- Title
    createRoomElements.title = UI.CreateText({
        x = centerX,
        y = 100,
        text = "CREATE ROOM",
        fontSize = 60,
        color = COLORS.PRIMARY_BLUE,
        menuGroup = menuGroup
    })
    
    -- Panel background
    createRoomElements.panel = UI.CreatePanel({
        x = centerX - 250,
        y = 180,
        width = 500,
        height = 450,
        backgroundColor = COLORS.PANEL_BG,
        modal = false,
        menuGroup = menuGroup
    })
    
    -- Room Name
    local fieldStartY = 220
    local fieldSpacing = 80
    
    createRoomElements.nameLabel = UI.CreateText({
        x = centerX - 200,
        y = fieldStartY,
        text = "Room Name:",
        fontSize = 22,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    createRoomElements.nameInput = UI.CreateInputField({
        x = centerX + 50,
        y = fieldStartY - 10,
        width = 200,
        height = 40,
        placeholder = "My Room",
        menuGroup = menuGroup
    })
    
    -- Max Players
    createRoomElements.maxPlayersLabel = UI.CreateText({
        x = centerX - 200,
        y = fieldStartY + fieldSpacing,
        text = "Max Players:",
        fontSize = 22,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    createRoomElements.maxPlayersDropdown = UI.CreateDropdown({
        x = centerX + 50,
        y = fieldStartY + fieldSpacing - 10,
        width = 200,
        options = {"2 Players", "3 Players", "4 Players"},
        selectedIndex = 1,
        onChange = "OnMaxPlayersChanged",
        menuGroup = menuGroup
    })
    
    -- Password (optional)
    createRoomElements.passwordLabel = UI.CreateText({
        x = centerX - 200,
        y = fieldStartY + fieldSpacing * 2,
        text = "Password:",
        fontSize = 22,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    createRoomElements.passwordInput = UI.CreateInputField({
        x = centerX + 50,
        y = fieldStartY + fieldSpacing * 2 - 10,
        width = 200,
        height = 40,
        placeholder = "(Optional)",
        menuGroup = menuGroup
    })
    
    -- Difficulty
    createRoomElements.difficultyLabel = UI.CreateText({
        x = centerX - 200,
        y = fieldStartY + fieldSpacing * 3,
        text = "Difficulty:",
        fontSize = 22,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    createRoomElements.difficultyDropdown = UI.CreateDropdown({
        x = centerX + 50,
        y = fieldStartY + fieldSpacing * 3 - 10,
        width = 200,
        options = {"Easy", "Normal", "Hard"},
        selectedIndex = 1,
        onChange = "OnDifficultyChanged",
        menuGroup = menuGroup
    })
    
    -- Buttons
    local buttonY = fieldStartY + fieldSpacing * 4 + 30
    
    createRoomElements.createBtn = UI.CreateButton({
        x = centerX - 110,
        y = buttonY,
        width = 180,
        height = 50,
        text = "CREATE",
        onClick = "OnConfirmCreateRoom",
        menuGroup = menuGroup
    })
    
    createRoomElements.backBtn = UI.CreateButton({
        x = centerX + 110,
        y = buttonY,
        width = 180,
        height = 50,
        text = "BACK",
        onClick = "OnBackToServerBrowser",
        menuGroup = menuGroup
    })
end

-- ============================================
-- LOBBY WAITING ROOM
-- ============================================
function CreateLobbyWaiting(centerX, centerY, width, height)
    local menuGroup = "lobby_waiting"
    
    -- Room name title (updated dynamically)
    lobbyElements.roomTitle = UI.CreateText({
        x = centerX,
        y = 80,
        text = "ROOM: [NAME]",
        fontSize = 50,
        color = COLORS.PRIMARY_BLUE,
        menuGroup = menuGroup
    })
    
    -- Room info
    lobbyElements.roomInfo = UI.CreateText({
        x = centerX,
        y = 140,
        text = "Waiting for players...",
        fontSize = 20,
        color = COLORS.LIGHT_GRAY,
        menuGroup = menuGroup
    })
    
    -- Players panel
    lobbyElements.playersPanel = UI.CreatePanel({
        x = centerX - 300,
        y = 180,
        width = 600,
        height = 350,
        backgroundColor = COLORS.PANEL_BG,
        modal = false,
        menuGroup = menuGroup
    })
    
    -- Players header
    lobbyElements.playersHeader = UI.CreateText({
        x = centerX,
        y = 200,
        text = "PLAYERS",
        fontSize = 28,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    -- Player slot references (created dynamically)
    lobbyElements.playerSlots = {}
    
    -- Create player slots dynamically (will be created when joining room)
    -- We'll create them in UpdateLobbyUI based on actual maxPlayers
    lobbyElements.playerSlots = {}
    
    -- Bottom buttons
    local buttonY = height - 150
    
    -- Start game button (host only) - centré à gauche
    lobbyElements.startBtn = UI.CreateButton({
        x = centerX - 110,
        y = buttonY,
        width = 180,
        height = 50,
        text = "START GAME",
        onClick = "OnStartGame",
        menuGroup = menuGroup
    })
    
    -- Leave button - centré à droite
    lobbyElements.leaveBtn = UI.CreateButton({
        x = centerX + 110,
        y = buttonY,
        width = 180,
        height = 50,
        text = "LEAVE",
        onClick = "OnLeaveLobby",
        menuGroup = menuGroup
    })
    
    -- Chat panel (prepared for future)
    lobbyElements.chatPanel = UI.CreatePanel({
        x = width - 350,
        y = 180,
        width = 300,
        height = 350,
        backgroundColor = COLORS.PANEL_BG,
        modal = false,
        menuGroup = menuGroup
    })
    
    lobbyElements.chatHeader = UI.CreateText({
        x = width - 200,
        y = 200,
        text = "CHAT",
        fontSize = 24,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    lobbyElements.chatInput = UI.CreateInputField({
        x = width - 250,
        y = 480,
        width = 200,
        height = 35,
        placeholder = "Type message...",
        onSubmit = "OnSendChatMessage",
        menuGroup = menuGroup
    })
    
    lobbyElements.chatSendBtn = UI.CreateButton({
        x = width - 100,
        y = 495,    
        width = 80,
        height = 35,
        text = "Send",
        onClick = "OnSendChatMessage",
        menuGroup = menuGroup
    })

    -- Chat history lines (initially empty)
    lobbyElements.chatLines = {}
    for i = 1, 10 do
        lobbyElements.chatLines[i] = UI.CreateText({
            x = width - 340,
            y = 230 + (i - 1) * 20,
            text = "",
            fontSize = 14,
            color = COLORS.LIGHT_GRAY,
            menuGroup = menuGroup
        })
    end
end

-- ============================================
-- SETTINGS MENU
-- ============================================
function CreateSettingsMenu(centerX, centerY, width, height)
    local menuGroup = "settings_menu"
    
    -- Title
    settingsElements.title = UI.CreateText({
        x = centerX,
        y = 100,
        text = "SETTINGS",
        fontSize = 60,
        color = COLORS.PRIMARY_BLUE,
        menuGroup = menuGroup
    })
    
    -- Panel background
    settingsElements.panel = UI.CreatePanel({
        x = centerX - 300,
        y = 180,
        width = 600,
        height = 500,
        backgroundColor = COLORS.PANEL_BG,
        modal = false,
        menuGroup = menuGroup
    })
    
    local startY = 230
    local spacing = 90
    
    -- Music Volume
    settingsElements.musicLabel = UI.CreateText({
        x = centerX - 200,
        y = startY,
        text = "Music Volume:",
        fontSize = 24,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    settingsElements.musicSlider = UI.CreateSlider({
        x = centerX + 50,
        y = startY - 5,
        width = 200,
        min = 0,
        max = 100,
        value = settingsData.musicVolume,
        onChange = "OnMusicVolumeChanged",
        menuGroup = menuGroup
    })
    
    settingsElements.musicValue = UI.CreateText({
        x = centerX + 280,
        y = startY,
        text = tostring(settingsData.musicVolume) .. "%",
        fontSize = 20,
        color = COLORS.LIGHT_GRAY,
        menuGroup = menuGroup
    })
    
    -- SFX Volume
    settingsElements.sfxLabel = UI.CreateText({
        x = centerX - 200,
        y = startY + spacing,
        text = "Sound Effects:",
        fontSize = 24,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    settingsElements.sfxSlider = UI.CreateSlider({
        x = centerX + 50,
        y = startY + spacing - 5,
        width = 200,
        min = 0,
        max = 100,
        value = settingsData.sfxVolume,
        onChange = "OnSFXVolumeChanged",
        menuGroup = menuGroup
    })
    
    settingsElements.sfxValue = UI.CreateText({
        x = centerX + 280,
        y = startY + spacing,
        text = tostring(settingsData.sfxVolume) .. "%",
        fontSize = 20,
        color = COLORS.LIGHT_GRAY,
        menuGroup = menuGroup
    })
    
    -- Resolution
    settingsElements.resLabel = UI.CreateText({
        x = centerX - 200,
        y = startY + spacing * 2,
        text = "Resolution:",
        fontSize = 24,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    settingsElements.resDropdown = UI.CreateDropdown({
        x = centerX + 50,
        y = startY + spacing * 2 - 10,
        width = 200,
        options = {"1920x1080", "1280x720", "1600x900"},
        selectedIndex = settingsData.resolution,
        onChange = "OnResolutionChanged",
        menuGroup = menuGroup
    })
    
    -- Fullscreen
    settingsElements.fullscreenLabel = UI.CreateText({
        x = centerX - 200,
        y = startY + spacing * 3,
        text = "Fullscreen:",
        fontSize = 24,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    settingsElements.fullscreenCheckbox = UI.CreateCheckbox({
        x = centerX + 50,
        y = startY + spacing * 3,
        label = "",
        checked = settingsData.fullscreen,
        onChange = "OnFullscreenChanged",
        menuGroup = menuGroup
    })
    
    -- Buttons
    local buttonY = startY + spacing * 4 + 30
    
    settingsElements.applyBtn = UI.CreateButton({
        x = centerX - 110,
        y = buttonY,
        width = 180,
        height = 50,
        text = "APPLY",
        onClick = "OnApplySettings",
        menuGroup = menuGroup
    })
    
    settingsElements.backBtn = UI.CreateButton({
        x = centerX + 110,
        y = buttonY,
        width = 180,
        height = 50,
        text = "BACK",
        onClick = "OnSettingsBack",
        menuGroup = menuGroup
    })
end

-- ============================================
-- PAUSE MENU
-- ============================================
function CreatePauseMenu(centerX, centerY, width, height)
    local menuGroup = "pause_menu"
    
    -- Semi-transparent overlay (more opaque for better visibility)
    pauseElements.overlay = UI.CreatePanel({
        x = 0,
        y = 0,
        width = width,
        height = height,
        backgroundColor = 0x000000CC,
        modal = true,
        menuGroup = menuGroup
    })
    
    -- Center panel (larger to accommodate new elements)
    pauseElements.panel = UI.CreatePanel({
        x = centerX - 250,
        y = centerY - 250,
        width = 500,
        height = 500,
        backgroundColor = COLORS.PANEL_BG,
        modal = false,
        menuGroup = menuGroup
    })
    
    -- BIG PAUSED Title
    pauseElements.bigTitle = UI.CreateText({
        x = centerX,
        y = centerY - 200,
        text = "GAME PAUSED",
        fontSize = 72,
        color = COLORS.PRIMARY_BLUE,
        menuGroup = menuGroup
    })
    
    -- Subtitle with instruction
    pauseElements.subtitle = UI.CreateText({
        x = centerX,
        y = centerY - 120,
        text = "Press ESC to resume",
        fontSize = 24,
        color = COLORS.LIGHT_GRAY,
        menuGroup = menuGroup
    })
    
    -- Buttons
    local buttonStartY = centerY - 40
    local buttonSpacing = BUTTON.HEIGHT + BUTTON.SPACING
    
    pauseElements.resumeBtn = UI.CreateButton({
        x = centerX,
        y = buttonStartY,
        width = BUTTON.WIDTH,
        height = BUTTON.HEIGHT,
        text = "RESUME",
        onClick = "OnResumeClicked",
        menuGroup = menuGroup
    })
    
    pauseElements.settingsBtn = UI.CreateButton({
        x = centerX,
        y = buttonStartY + buttonSpacing,
        width = BUTTON.WIDTH,
        height = BUTTON.HEIGHT,
        text = "SETTINGS",
        onClick = "OnPauseSettingsClicked",
        menuGroup = menuGroup
    })
    
    -- Quit to menu button (abandon game)
    pauseElements.quitGameBtn = UI.CreateButton({
        x = centerX,
        y = buttonStartY + buttonSpacing * 2,
        width = BUTTON.WIDTH,
        height = BUTTON.HEIGHT,
        text = "QUIT TO MENU",
        onClick = "OnQuitToMenuClicked",
        menuGroup = menuGroup
    })
    
    -- Warning text for quit button
    pauseElements.quitWarning = UI.CreateText({
        x = centerX,
        y = buttonStartY + buttonSpacing * 2 + 60,
        text = "(Progress will be lost)",
        fontSize = 16,
        color = 0xFF6666FF,
        menuGroup = menuGroup
    })
end

-- ============================================
-- MAIN INITIALIZATION FUNCTION
-- ============================================
function InitUI(width, height)
    screenWidth = width or 1920
    screenHeight = height or 1080
    
    local centerX = screenWidth / 2
    local centerY = screenHeight / 2
    
    print("[UI] Initializing UI with resolution: " .. screenWidth .. "x" .. screenHeight)
    
    -- Create all menus
    CreateMainMenu(centerX, centerY, screenWidth, screenHeight)
    CreateServerBrowser(centerX, centerY, screenWidth, screenHeight)
    CreateRoomCreation(centerX, centerY, screenWidth, screenHeight)
    CreateLobbyWaiting(centerX, centerY, screenWidth, screenHeight)
    CreateSettingsMenu(centerX, centerY, screenWidth, screenHeight)
    CreatePauseMenu(centerX, centerY, screenWidth, screenHeight)
    
    -- Hide all menus initially
    UI.HideAllMenus()
    
    -- Show main menu
    UI.ShowMenu("main_menu")
    UI.SetActiveMenu("main_menu")
    
    print("[UI] UI initialization complete!")
end

-- ============================================
-- MAIN MENU CALLBACKS
-- ============================================
function OnPlayClicked()
    print("[UI] Play clicked - starting game")
    UI.HideAllMenus()
    GameState.Set("Playing")
end

function OnMultiplayerClicked()
    print("[UI] Multiplayer clicked")
    UI.HideAllMenus()
    UI.ShowMenu("server_browser")
    UI.SetActiveMenu("server_browser")
    RefreshServerList()
end

function OnSettingsClicked()
    print("[UI] Settings clicked")
    UI.HideAllMenus()
    UI.ShowMenu("settings_menu")
    UI.SetActiveMenu("settings_menu")
end

function OnQuitClicked()
    print("[UI] Quit clicked")
    os.exit(0)
end

function OnBackToMainMenu()
    print("[UI] Back to main menu")
    UI.HideAllMenus()
    UI.ShowMenu("main_menu")
    UI.SetActiveMenu("main_menu")
    GameState.Set("MainMenu")
end

-- ============================================
-- SERVER BROWSER CALLBACKS
-- ============================================
function OnRefreshServers()
    print("[UI] Refreshing server list...")
    RefreshServerList()
end

function OnJoinByIP()
    -- Prevent duplicate connect attempts
    if lobbyData.joiningRoom then
        print("[UI] Already attempting to connect, please wait...")
        return
    end

    local ip = UI.GetInputText(serverBrowserElements.ipInput)
    if not ip or ip == "" then
        print("[UI] No IP entered")
        return
    end

    -- If format is host:port, attempt to connect the Network client at runtime
    local host, port = string.match(ip, "^([^:]+):?(%d*)$")
    if host and port and port ~= "" then
        print("[UI] Connecting to server at: " .. host .. ":" .. port)
        -- mark joining to avoid double-clicks
        lobbyData.joiningRoom = true
        lobbyData.joinTimeout = os.time()
        if serverBrowserElements.connectedText then
            UI.SetText(serverBrowserElements.connectedText, "Connecting...")
            UI.SetVisible(serverBrowserElements.connectedText, true)
        end
        if Network and Network.Connect then
            Network.Connect(host, tonumber(port))
            -- After connecting, request the room list to populate UI
            RefreshServerList()
        else
            print("[UI] WARNING: Network.Connect not available in bindings")
            lobbyData.joiningRoom = false
        end
    else
        -- Fallback: try join by room id if it's a number
        local n = tonumber(ip)
        if n then
            print("[UI] Joining room by ID: " .. n)
            JoinRoom(n)
        else
            print("[UI] Invalid input. Enter host:port or room ID.")
        end
    end
end

-- Callback from C++ when a connection is established
function OnConnected(host, port)
    print("[UI] OnConnected callback: " .. tostring(host) .. ":" .. tostring(port))
    -- IMPORTANT: Reset joiningRoom flag to allow joining rooms
    lobbyData.joiningRoom = false
    if serverBrowserElements.connectedText then
        UI.SetText(serverBrowserElements.connectedText, "Connected: " .. tostring(host) .. ":" .. tostring(port))
        UI.SetVisible(serverBrowserElements.connectedText, true)
    end
    -- Refresh room list after connection
    RefreshServerList()
end

function OnCreateRoomClicked()
    print("[UI] Opening create room menu")
    UI.HideAllMenus()
    UI.ShowMenu("create_room_menu")
    UI.SetActiveMenu("create_room_menu")
end

function OnBackToServerBrowser()
    print("[UI] Back to server browser")
    UI.HideAllMenus()
    UI.ShowMenu("server_browser")
    UI.SetActiveMenu("server_browser")
end

-- ============================================
-- CREATE ROOM CALLBACKS
-- ============================================
function OnMaxPlayersChanged(index)
    print("[UI] Max players changed to: " .. (index + 1))
end

function OnDifficultyChanged(index)
    local difficulties = {"Easy", "Normal", "Hard"}
    print("[UI] Difficulty changed to: " .. difficulties[index + 1])
end

function OnConfirmCreateRoom()
    local roomName = UI.GetInputText(createRoomElements.nameInput)
    local maxPlayers = UI.GetDropdownIndex(createRoomElements.maxPlayersDropdown) + 2
    local password = UI.GetInputText(createRoomElements.passwordInput)
    local difficulty = UI.GetDropdownIndex(createRoomElements.difficultyDropdown)
    
    if roomName == "" or roomName == nil then
        roomName = "Room " .. os.time()
    end
    
    local config = {
        name = roomName,
        maxPlayers = maxPlayers,
        password = password,
        difficulty = difficulty
    }
    
    print("[UI] Creating room: " .. roomName)
    CreateRoom(config)
    
    -- Go to lobby
    lobbyData.isHost = true
    UpdateLobbyUI(roomName)
    UI.HideAllMenus()
    UI.ShowMenu("lobby_waiting")
    UI.SetActiveMenu("lobby_waiting")
end

-- ============================================
-- LOBBY CALLBACKS
-- ============================================
-- Players are automatically ready when they join, no toggle needed

function OnStartGame()
    if lobbyData.isHost then
        print("[UI] Host starting game")
        StartGame()
    end
end

function OnLeaveLobby()
    print("[UI] Leaving lobby")
    lobbyData.isHost = false
    lobbyData.isReady = false
    lobbyData.currentRoom = nil
    LeaveRoom()
    
    UI.HideAllMenus()
    UI.ShowMenu("server_browser")
    UI.SetActiveMenu("server_browser")
    
    -- Clear chat history
    lobbyData.chatHistory = {}
    UpdateChatUI()
end

function OnSendChatMessage(optMsg)
    local message = optMsg
    
    -- If called from button (no arg) or if arg is empty, get from input
    if not message or message == "" then
        message = UI.GetInputText(lobbyElements.chatInput)
    end

    if message and message ~= "" then
        print("[UI] Sending chat: " .. message)
        
        -- Get current room ID
        local roomId = 0
        if lobbyData.currentRoom and lobbyData.currentRoom.id then
            roomId = lobbyData.currentRoom.id
        else
            print("[UI] WARNING: No current room ID found!")
        end
        
        SendChatMessage(message, roomId)
        UI.SetInputText(lobbyElements.chatInput, "")
    end
end

-- ============================================
-- SETTINGS CALLBACKS
-- ============================================

-- Flags to prevent recursive/infinite calls
local isUpdatingMusicVolume = false
local isUpdatingSFXVolume = false

function OnMusicVolumeChanged(value)
    -- Prevent infinite loop from recursive calls
    if isUpdatingMusicVolume then return end
    
    local newVolume = math.floor(value)
    -- Only update if value actually changed
    if newVolume == settingsData.musicVolume then return end
    
    isUpdatingMusicVolume = true
    settingsData.musicVolume = newVolume
    UI.SetText(settingsElements.musicValue, tostring(settingsData.musicVolume) .. "%")
    
    -- Update actual music volume in C++ (new audio system)
    if Audio and Audio.SetMusicVolume then
        Audio.SetMusicVolume(settingsData.musicVolume)
        print("[UI] 🎵 Music volume: " .. settingsData.musicVolume .. "%")
    elseif SetMenuMusicVolume ~= nil then
        -- Fallback to legacy function
        SetMenuMusicVolume(settingsData.musicVolume)
        print("[UI] Music volume changed to: " .. settingsData.musicVolume .. "%")
    else
        print("[UI] WARNING: No music volume function available!")
    end
    
    isUpdatingMusicVolume = false
end

function OnSFXVolumeChanged(value)
    -- Prevent infinite loop from recursive calls
    if isUpdatingSFXVolume then return end
    
    local newVolume = math.floor(value)
    -- Only update if value actually changed
    if newVolume == settingsData.sfxVolume then return end
    
    isUpdatingSFXVolume = true
    settingsData.sfxVolume = newVolume
    UI.SetText(settingsElements.sfxValue, tostring(settingsData.sfxVolume) .. "%")
    
    -- Update actual SFX volume in C++ (new audio system)
    if Audio and Audio.SetSFXVolume then
        Audio.SetSFXVolume(settingsData.sfxVolume)
        print("[UI] 🔊 SFX volume: " .. settingsData.sfxVolume .. "%")
    else
        print("[UI] SFX volume: " .. settingsData.sfxVolume .. "%")
    end
    
    isUpdatingSFXVolume = false
end

-- Flag to prevent resolution change loops
local isUpdatingResolution = false

function OnResolutionChanged(index)
    if index == nil then return end
    
    -- Prevent infinite loop from recursive calls
    if isUpdatingResolution then return end
    
    local newIndex = math.floor(index)
    -- Only update if value actually changed
    if newIndex == settingsData.resolution then return end
    
    isUpdatingResolution = true
    settingsData.resolution = newIndex
    
    local resolutions = {"1920x1080", "1280x720", "1600x900"}
    local resIndex = newIndex + 1
    if resIndex >= 1 and resIndex <= #resolutions then
        print("[UI] Resolution changed to: " .. resolutions[resIndex])
        
        -- Auto-enable fullscreen for 1920x1080 (index 0)
        if newIndex == 0 then
            settingsData.fullscreen = true
            print("[UI] Auto-enabled fullscreen for 1920x1080")
            -- Update checkbox visual if it exists
            if settingsElements and settingsElements.fullscreenCheckbox then
                UI.SetCheckboxChecked(settingsElements.fullscreenCheckbox, true)
            end
        end
    end
    
    isUpdatingResolution = false
end

-- Flag to prevent fullscreen change loops
local isUpdatingFullscreen = false

function OnFullscreenChanged(checked)
    if checked == nil then return end
    
    -- Prevent infinite loop from recursive calls
    if isUpdatingFullscreen then return end
    
    local newValue = (checked == true or checked == 1 or checked == 1.0)
    -- Only update if value actually changed
    if newValue == settingsData.fullscreen then return end
    
    isUpdatingFullscreen = true
    settingsData.fullscreen = newValue
    print("[UI] Fullscreen changed to: " .. tostring(settingsData.fullscreen))
    isUpdatingFullscreen = false
end

-- Flags to prevent infinite loop in OnApplySettings
local isApplyingSettings = false
local lastAppliedResolution = -1
local lastAppliedFullscreen = nil

function OnApplySettings()
    -- Prevent infinite loop
    if isApplyingSettings then 
        print("[UI] OnApplySettings already in progress, skipping")
        return 
    end
    
    -- Check if resolution/fullscreen actually changed
    local resIndex = settingsData.resolution or 0
    local fullscreen = settingsData.fullscreen or false
    
    if resIndex == lastAppliedResolution and fullscreen == lastAppliedFullscreen then
        print("[UI] Settings unchanged, only saving to file")
        -- Still save other settings (volume, etc.)
        if SaveUserSettingsToFile ~= nil then
            SaveUserSettingsToFile()
        end
        return
    end
    
    isApplyingSettings = true
    
    print("[UI] 💾 Applying settings...")
    print("  Music: " .. tostring(settingsData.musicVolume or 70) .. "%")
    print("  SFX: " .. tostring(settingsData.sfxVolume or 80) .. "%")
    print("  Resolution: " .. tostring(resIndex))
    print("  Fullscreen: " .. tostring(fullscreen))
    
    -- Apply resolution and fullscreen changes
    if ApplyResolution ~= nil then
        ApplyResolution(resIndex, fullscreen)
        
        -- Remember what we applied to prevent duplicates
        lastAppliedResolution = resIndex
        lastAppliedFullscreen = fullscreen
        
        print("[UI] ✓ Resolution applied!")
    else
        print("[UI] Warning: ApplyResolution not available")
    end
    
    -- Call C++ to save settings to user_settings.json
    if SaveUserSettingsToFile ~= nil then
        SaveUserSettingsToFile()
    end
    
    print("[UI] ✓ Settings saved!")
    
    isApplyingSettings = false
end

function OnSettingsBack()
    local currentMenu = UI.GetActiveMenu()
    UI.HideAllMenus()
    
    -- Return to appropriate menu based on context
    if GameState.IsPaused() then
        UI.ShowMenu("pause_menu")
        UI.SetActiveMenu("pause_menu")
    else
        UI.ShowMenu("main_menu")
        UI.SetActiveMenu("main_menu")
    end
end

-- ============================================
-- PAUSE MENU CALLBACKS
-- ============================================
function OnResumeClicked()
    print("[UI] Resuming game")
    UI.HideAllMenus()
    GameState.Set("Playing")
end

function OnPauseSettingsClicked()
    print("[UI] Opening settings from pause")
    UI.HideAllMenus()
    UI.ShowMenu("settings_menu")
    UI.SetActiveMenu("settings_menu")
end

function OnPauseMainMenuClicked()
    print("[UI] Returning to main menu from pause")
    UI.HideAllMenus()
    UI.ShowMenu("main_menu")
    UI.SetActiveMenu("main_menu")
    GameState.Set("MainMenu")
end

-- New callback for quitting to menu (abandoning game)
function OnQuitToMenuClicked()
    print("[UI] Quitting game and returning to main menu")
    UI.HideAllMenus()
    
    -- Stop game music
    if Audio and Audio.StopMusic then
        Audio.StopMusic()
    end
    
    UI.ShowMenu("main_menu")
    UI.SetActiveMenu("main_menu")
    GameState.Set("MainMenu")
end

-- ============================================
-- NETWORK STUB FUNCTIONS (for future integration)
-- ============================================
function RefreshServerList()
    print("[UI] Refreshing server list...")
    -- Call C++ network binding to request room list from server
    if Network and Network.RequestRoomList then
        Network.RequestRoomList()
    else
        print("[UI] WARNING: Network.RequestRoomList not available!")
        lobbyData.rooms = {}
        if serverBrowserElements.noServersText then
            UI.SetVisible(serverBrowserElements.noServersText, true)
        end
    end
end

function JoinRoom(roomIdOrIP)
    -- Protection contre les double-clics
    if lobbyData.joiningRoom then
        print("[UI] Already joining a room, please wait...")
        return
    end
    
    print("[UI] Joining room: " .. tostring(roomIdOrIP))
    lobbyData.joiningRoom = true  -- Marquer qu'on est en train de rejoindre
    
    -- Timeout de sécurité : reset après 5 secondes
    local joinTime = os.time()
    lobbyData.joinTimeout = joinTime
    
    -- Call C++ network binding to join a room
    if Network and Network.JoinRoom then
        if type(roomIdOrIP) == "number" then
            Network.JoinRoom(roomIdOrIP)
        else
            print("[UI] Cannot join by IP yet (only room IDs supported)")
            lobbyData.joiningRoom = false  -- Reset si erreur
        end
    else
        print("[UI] WARNING: Network.JoinRoom not available!")
        lobbyData.joiningRoom = false  -- Reset si erreur
    end
end

function CreateRoom(config)
    print("[UI] Creating room")
    print("  Name: " .. tostring(config.name))
    print("  Max Players: " .. tostring(config.maxPlayers))
    print("  Difficulty: " .. tostring(config.difficulty))
    -- Call C++ network binding to create a room
    if Network and Network.CreateRoom then
        Network.CreateRoom(
            config.name or "New Room",
            config.maxPlayers or 4,
            config.password or "",
            config.difficulty or 1
        )
    else
        print("[UI] WARNING: Network.CreateRoom not available!")
    end
end

function SetReady(ready)
    print("[UI] Set ready: " .. tostring(ready))
    -- Call C++ network binding to set ready state
    if Network and Network.SetPlayerReady then
        Network.SetPlayerReady(ready)
    else
        print("[UI] WARNING: Network.SetPlayerReady not available!")
    end
end

function StartGame()
    print("[UI] Starting game...")
    -- Call C++ network binding to start the game (host only)
    if Network and Network.StartGame then
        Network.StartGame()
    else
        print("[UI] WARNING: Network.StartGame not available!")
        -- Fallback: start solo game
        UI.HideAllMenus()
        GameState.Set("Playing")
    end
end

function SendChatMessage(message, roomId)
    print("[UI] Sending chat message to room " .. tostring(roomId) .. ": " .. message)
    if Network and Network.SendChatMessage then
        Network.SendChatMessage(message, roomId)
    else
        print("[UI] WARNING: Network.SendChatMessage not available!")
    end
end

function LeaveRoom()
    print("[UI] Leaving room...")
    -- Call C++ network binding to leave the current room
    if Network and Network.LeaveRoom then
        Network.LeaveRoom()
    else
        print("[UI] WARNING: Network.LeaveRoom not available!")
    end
end

-- ============================================
-- NETWORK CALLBACKS (called by C++ later)
-- ============================================
function OnServerListUpdated(servers)
    print("[UI] Server list updated with " .. #servers .. " servers")
    lobbyData.rooms = servers
    UpdateServerListUI()
end

function OnRoomJoined(roomInfo)
    print("[UI] Joined room: " .. roomInfo.name)
    lobbyData.joiningRoom = false  -- Reset le flag de join
    lobbyData.currentRoom = {
        id = roomInfo.id,
        name = roomInfo.name,
        isHost = roomInfo.isHost or false,
        maxPlayers = roomInfo.maxPlayers or 4
    }
    lobbyData.isHost = roomInfo.isHost or false
    -- Players are automatically ready when joining
    lobbyData.isReady = true
    if Network and Network.SetPlayerReady then
        Network.SetPlayerReady(true)
        print("[UI] Auto-ready: true")
    end
    UpdateLobbyUI(roomInfo.name)
    UI.HideAllMenus()
    UI.ShowMenu("lobby_waiting")
    UI.SetActiveMenu("lobby_waiting")
end

function OnPlayerJoined(playerInfo)
    print("[UI] Player joined: " .. playerInfo.name)
    table.insert(lobbyData.players, playerInfo)
    UpdatePlayerListUI()
end

function OnPlayerLeft(playerId)
    print("[UI] Player left: " .. playerId)
    for i, player in ipairs(lobbyData.players) do
        if player.id == playerId then
            table.remove(lobbyData.players, i)
            break
        end
    end
    UpdatePlayerListUI()
end

function OnPlayerReadyChanged(playerId, ready)
    print("[UI] Player " .. playerId .. " ready: " .. tostring(ready))
    for _, player in ipairs(lobbyData.players) do
        if player.id == playerId then
            player.ready = ready
            break
        end
    end
    UpdatePlayerListUI()
end

function OnRoomPlayersUpdated(players)
    print("[UI] Room players updated: " .. #players .. " players")
    lobbyData.players = {}
    
    for i, player in ipairs(players) do
        table.insert(lobbyData.players, {
            id = player.id,
            name = player.name,
            isHost = player.isHost,
            ready = player.ready
        })
        print("[UI]   - " .. player.name .. (player.isHost and " (HOST)" or ""))
    end
    
    UpdatePlayerListUI()
end

function OnGameStarting(countdown)
    print("[UI] Game starting in " .. countdown .. " seconds")
    
    -- Cacher le menu du lobby pour laisser place au jeu
    UI.HideMenu("lobby_waiting")
    UI.HideMenu("server_browser")
    UI.HideMenu("create_room_menu")
    UI.HideMenu("main_menu")
    
    -- Afficher un compte à rebours si besoin
    if lobbyElements.roomInfo then
        UI.SetText(lobbyElements.roomInfo, "Starting in " .. countdown .. "...")
    end
    
    print("[UI] Lobby menus hidden, game starting...")
end

function OnChatMessage(sender, message)
    print("[Chat] " .. sender .. ": " .. message)
    
    if not lobbyData.chatHistory then
        lobbyData.chatHistory = {}
    end
    
    table.insert(lobbyData.chatHistory, {
        sender = sender,
        message = message,
        time = os.date("%H:%M")
    })
    
    -- Keep only last 10 messages
    if #lobbyData.chatHistory > 10 then
        table.remove(lobbyData.chatHistory, 1)
    end
    
    UpdateChatUI()
end

function UpdateChatUI()
    if not lobbyElements.chatLines then return end
    
    local history = lobbyData.chatHistory or {}
    local startIndex = math.max(1, #history - 9)
    local lineIndex = 1
    
    -- Clear all lines first
    for i = 1, 10 do
        if lobbyElements.chatLines[i] then
            UI.SetText(lobbyElements.chatLines[i], "")
        end
    end
    
    -- Populate with history
    for i = 1, #history do
        local entry = history[i]
        local displayString = "[" .. (entry.time or "") .. "] " .. entry.sender .. ": " .. entry.message
        
        -- Use lines from bottom up or top down? Let's do top down for now
        if lobbyElements.chatLines[i] then
           UI.SetText(lobbyElements.chatLines[i], displayString)
        end
    end
end

-- ============================================
-- UI UPDATE HELPERS
-- ============================================
function UpdateServerListUI()
    -- NOUVEAU: Ne pas mettre à jour si on est déjà dans une room !
    if lobbyData.currentRoom then
        print("[UI] Ignoring server list update - already in a room")
        return
    end
    
    -- Nettoyer les anciennes callbacks
    if serverBrowserElements.serverEntries then
        for _, entry in ipairs(serverBrowserElements.serverEntries) do
            if entry.roomId then
                _G["OnRoomSelected_" .. entry.roomId] = nil  -- Supprime la callback
            end
        end
    end
    serverBrowserElements.serverEntries = {}
    
    -- Create new server entries dynamically
    local startY = 250
    local spacing = 70
    local centerX = 960  -- Assuming 1920x1080
    
    for i, room in ipairs(lobbyData.rooms) do
        if i <= 6 then  -- Limit to 6 rooms visible
            local y = startY + (i - 1) * spacing
            
            -- Create room button
            local buttonText = room.name .. " (" .. room.currentPlayers .. "/" .. room.maxPlayers .. ")"
            local roomButton = UI.CreateButton({
                x = centerX,
                y = y,
                width = 600,
                height = 55,
                text = buttonText,
                onClick = "OnRoomSelected_" .. room.id,
                menuGroup = "server_browser"
            })
            
            -- Store the button and create the callback
            table.insert(serverBrowserElements.serverEntries, {
                button = roomButton,
                roomId = room.id
            })
            
            -- Create global callback function for this room
            _G["OnRoomSelected_" .. room.id] = function()
                print("[UI] Selected room: " .. room.name .. " (ID: " .. room.id .. ")")
                JoinRoom(room.id)
            end
        end
    end
    
    print("[UI] Updated server list with " .. #lobbyData.rooms .. " rooms")
end

function UpdateLobbyUI(roomName)
    if lobbyElements.roomTitle then
        UI.SetText(lobbyElements.roomTitle, "ROOM: " .. roomName)
    end
    
    -- Create slots dynamically based on maxPlayers
    local maxPlayers = (lobbyData.currentRoom and lobbyData.currentRoom.maxPlayers) or 4
    local menuGroup = "lobby_waiting"
    local centerX = 960
    
    -- Create slots if they don't exist or if count changed
    if #lobbyElements.playerSlots ~= maxPlayers then
        lobbyElements.playerSlots = {}
        for i = 1, maxPlayers do
            local slotY = 250 + (i - 1) * 70
            lobbyElements.playerSlots[i] = {
                panel = UI.CreatePanel({
                    x = centerX - 250,
                    y = slotY,
                    width = 500,
                    height = 60,
                    backgroundColor = 0x1E90FF44,
                    modal = false,
                    menuGroup = menuGroup
                }),
                nameText = UI.CreateText({
                    x = centerX - 150,
                    y = slotY + 20,
                    text = "Empty Slot",
                    fontSize = 22,
                    color = COLORS.GRAY,
                    menuGroup = menuGroup
                }),
                statusText = UI.CreateText({
                    x = centerX + 150,
                    y = slotY + 20,
                    text = "",
                    fontSize = 20,
                    color = COLORS.GRAY,
                    menuGroup = menuGroup
                })
            }
        end
    end
    
    -- Show/hide host-specific buttons
    if lobbyElements.startBtn then
        UI.SetVisible(lobbyElements.startBtn, lobbyData.isHost)
    end
    
    -- Players are automatically ready
    lobbyData.isReady = true
end

function UpdatePlayerListUI()
    -- Update player slots
    for i = 1, 4 do
        local slot = lobbyElements.playerSlots[i]
        if slot then
            if i <= #lobbyData.players then
                local player = lobbyData.players[i]
                UI.SetText(slot.nameText, player.name)
                
                -- All players are automatically ready
                local statusText = "READY"
                if player.isHost then
                    statusText = "HOST"
                end
                UI.SetText(slot.statusText, statusText)
            else
                UI.SetText(slot.nameText, "Empty Slot")
                UI.SetText(slot.statusText, "")
            end
        end
    end
end

-- ============================================
-- ANIMATION UPDATE (call from C++ each frame)
-- ============================================
function UpdateUI(deltaTime)
    -- Title pulse animation
    if animations.titlePulse then
        animations.titlePulse.timer = animations.titlePulse.timer + deltaTime * animations.titlePulse.speed
        
        -- Calculate scale using sine wave
        animations.titlePulse.scale = 1.0 + math.sin(animations.titlePulse.timer) * 0.05
        
        -- Note: Would need UI.SetScale() function to actually apply this
        -- For now, this is prepared for future implementation
    end
    
    -- Timeout de sécurité pour le join
    if lobbyData.joiningRoom and lobbyData.joinTimeout then
        local elapsed = os.time() - lobbyData.joinTimeout
        if elapsed > 5 then  -- 5 secondes timeout
            print("[UI] Join timeout - resetting flag")
            lobbyData.joiningRoom = false
            lobbyData.joinTimeout = nil
        end
    end
end

-- ============================================
-- KEYBOARD NAVIGATION HELPER
-- ============================================
function HandleEscape()
    local activeMenu = UI.GetActiveMenu()
    
    if activeMenu == "server_browser" or activeMenu == "settings_menu" then
        if not GameState.IsPaused() then
            OnBackToMainMenu()
        else
            UI.HideAllMenus()
            UI.ShowMenu("pause_menu")
            UI.SetActiveMenu("pause_menu")
        end
    elseif activeMenu == "create_room_menu" then
        OnBackToServerBrowser()
    elseif activeMenu == "lobby_waiting" then
        OnLeaveLobby()
    elseif activeMenu == "pause_menu" then
        OnResumeClicked()
    elseif GameState.IsPlaying() then
        GameState.TogglePause()
        UI.ShowMenu("pause_menu")
        UI.SetActiveMenu("pause_menu")
    end
end

print("[UI Script] ui_init.lua loaded successfully!")
