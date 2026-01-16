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
    selectedServerId = nil
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
    
    -- Server list header
    serverBrowserElements.serverHeader = UI.CreateText({
        x = centerX,
        y = 170,
        text = "AVAILABLE SERVERS",
        fontSize = 24,
        color = COLORS.WHITE,
        menuGroup = menuGroup
    })
    
    -- Placeholder for server list (will be populated dynamically)
    serverBrowserElements.noServersText = UI.CreateText({
        x = centerX,
        y = 350,
        text = "No servers found. Click REFRESH or CREATE ROOM.",
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
    
    -- Create 4 player slots
    for i = 1, 4 do
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
    
    -- Bottom buttons
    local buttonY = height - 150
    
    -- Ready button (for non-hosts)
    lobbyElements.readyBtn = UI.CreateButton({
        x = centerX - 220,
        y = buttonY,
        width = 180,
        height = 50,
        text = "READY",
        onClick = "OnToggleReady",
        menuGroup = menuGroup
    })
    
    -- Start game button (host only)
    lobbyElements.startBtn = UI.CreateButton({
        x = centerX,
        y = buttonY,
        width = 180,
        height = 50,
        text = "START GAME",
        onClick = "OnStartGame",
        menuGroup = menuGroup
    })
    
    -- Leave button
    lobbyElements.leaveBtn = UI.CreateButton({
        x = centerX + 220,
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
    
    -- Semi-transparent overlay
    pauseElements.overlay = UI.CreatePanel({
        x = 0,
        y = 0,
        width = width,
        height = height,
        backgroundColor = 0x000000AA,
        modal = true,
        menuGroup = menuGroup
    })
    
    -- Center panel
    pauseElements.panel = UI.CreatePanel({
        x = centerX - 200,
        y = centerY - 180,
        width = 400,
        height = 360,
        backgroundColor = COLORS.PANEL_BG,
        modal = false,
        menuGroup = menuGroup
    })
    
    -- Title
    pauseElements.title = UI.CreateText({
        x = centerX,
        y = centerY - 140,
        text = "PAUSED",
        fontSize = 50,
        color = COLORS.PRIMARY_BLUE,
        menuGroup = menuGroup
    })
    
    -- Buttons
    local buttonStartY = centerY - 50
    
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
        y = buttonStartY + BUTTON.HEIGHT + BUTTON.SPACING,
        width = BUTTON.WIDTH,
        height = BUTTON.HEIGHT,
        text = "SETTINGS",
        onClick = "OnPauseSettingsClicked",
        menuGroup = menuGroup
    })
    
    pauseElements.mainMenuBtn = UI.CreateButton({
        x = centerX,
        y = buttonStartY + (BUTTON.HEIGHT + BUTTON.SPACING) * 2,
        width = BUTTON.WIDTH,
        height = BUTTON.HEIGHT,
        text = "MAIN MENU",
        onClick = "OnPauseMainMenuClicked",
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
    local ip = UI.GetInputText(serverBrowserElements.ipInput)
    if ip and ip ~= "" then
        print("[UI] Joining server at: " .. ip)
        JoinRoom(ip)
    else
        print("[UI] No IP entered")
    end
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
function OnToggleReady()
    lobbyData.isReady = not lobbyData.isReady
    print("[UI] Ready status: " .. tostring(lobbyData.isReady))
    
    -- Update button text
    if lobbyData.isReady then
        UI.SetText(lobbyElements.readyBtn, "NOT READY")
    else
        UI.SetText(lobbyElements.readyBtn, "READY")
    end
    
    SetReady(lobbyData.isReady)
end

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
end

function OnSendChatMessage()
    local message = UI.GetInputText(lobbyElements.chatInput)
    if message and message ~= "" then
        print("[UI] Sending chat: " .. message)
        SendChatMessage(message)
        UI.SetInputText(lobbyElements.chatInput, "")
    end
end

-- ============================================
-- SETTINGS CALLBACKS
-- ============================================
function OnMusicVolumeChanged(value)
    settingsData.musicVolume = math.floor(value)
    UI.SetText(settingsElements.musicValue, tostring(settingsData.musicVolume) .. "%")
    print("[UI] Music volume: " .. settingsData.musicVolume)
end

function OnSFXVolumeChanged(value)
    settingsData.sfxVolume = math.floor(value)
    UI.SetText(settingsElements.sfxValue, tostring(settingsData.sfxVolume) .. "%")
    print("[UI] SFX volume: " .. settingsData.sfxVolume)
end

function OnResolutionChanged(index)
    if index == nil then return end
    settingsData.resolution = index
    local resolutions = {"1920x1080", "1280x720", "1600x900"}
    local resIndex = math.floor(index) + 1
    if resIndex >= 1 and resIndex <= #resolutions then
        print("[UI] Resolution: " .. resolutions[resIndex])
    end
end

function OnFullscreenChanged(checked)
    if checked ~= nil then
        settingsData.fullscreen = (checked == true or checked == 1 or checked == 1.0)
    end
    print("[UI] Fullscreen: " .. tostring(settingsData.fullscreen))
end

function OnApplySettings()
    print("[UI] Applying settings...")
    -- Settings will be applied by C++ callbacks
    print("  Music: " .. tostring(settingsData.musicVolume or 70))
    print("  SFX: " .. tostring(settingsData.sfxVolume or 80))
    print("  Resolution: " .. tostring(settingsData.resolution or 0))
    print("  Fullscreen: " .. tostring(settingsData.fullscreen or false))
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

-- ============================================
-- NETWORK STUB FUNCTIONS (for future integration)
-- ============================================
function RefreshServerList()
    print("[Network Stub] RefreshServerList called")
    -- Will be implemented when network is ready
    lobbyData.rooms = {}
    
    -- For testing, show placeholder
    if serverBrowserElements.noServersText then
        UI.SetVisible(serverBrowserElements.noServersText, true)
    end
end

function JoinRoom(roomIdOrIP)
    print("[Network Stub] JoinRoom called with: " .. tostring(roomIdOrIP))
    -- Will be implemented when network is ready
end

function CreateRoom(config)
    print("[Network Stub] CreateRoom called")
    print("  Name: " .. tostring(config.name))
    print("  Max Players: " .. tostring(config.maxPlayers))
    print("  Difficulty: " .. tostring(config.difficulty))
    -- Will be implemented when network is ready
end

function SetReady(ready)
    print("[Network Stub] SetReady: " .. tostring(ready))
    -- Will be implemented when network is ready
end

function StartGame()
    print("[Network Stub] StartGame called")
    -- Will be implemented when network is ready
    UI.HideAllMenus()
    GameState.Set("Playing")
end

function SendChatMessage(message)
    print("[Network Stub] SendChatMessage: " .. message)
    -- Will be implemented when network is ready
end

function LeaveRoom()
    print("[Network Stub] LeaveRoom called")
    -- Will be implemented when network is ready
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
    lobbyData.currentRoom = roomInfo
    lobbyData.isHost = roomInfo.isHost
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

function OnGameStarting(countdown)
    print("[UI] Game starting in " .. countdown .. " seconds")
    UI.SetText(lobbyElements.roomInfo, "Starting in " .. countdown .. "...")
end

function OnChatMessage(sender, message)
    print("[Chat] " .. sender .. ": " .. message)
    -- Will update chat UI when implemented
end

-- ============================================
-- UI UPDATE HELPERS
-- ============================================
function UpdateServerListUI()
    -- Hide "no servers" text if we have servers
    if serverBrowserElements.noServersText then
        UI.SetVisible(serverBrowserElements.noServersText, #lobbyData.rooms == 0)
    end
    
    -- Update server entries (would need dynamic creation)
    -- For now, this is a placeholder
end

function UpdateLobbyUI(roomName)
    if lobbyElements.roomTitle then
        UI.SetText(lobbyElements.roomTitle, "ROOM: " .. roomName)
    end
    
    -- Show/hide host-specific buttons
    if lobbyElements.startBtn then
        UI.SetVisible(lobbyElements.startBtn, lobbyData.isHost)
    end
    
    -- Reset ready button
    if lobbyElements.readyBtn then
        UI.SetText(lobbyElements.readyBtn, "READY")
        UI.SetVisible(lobbyElements.readyBtn, not lobbyData.isHost)
    end
    
    lobbyData.isReady = false
end

function UpdatePlayerListUI()
    -- Update player slots
    for i = 1, 4 do
        local slot = lobbyElements.playerSlots[i]
        if slot then
            if i <= #lobbyData.players then
                local player = lobbyData.players[i]
                UI.SetText(slot.nameText, player.name)
                
                local statusText = player.ready and "READY" or "NOT READY"
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
