/**
 * MultiplayerMenuState.cpp - Multiplayer Lobby Implementation (Phase 5)
 */

#include "states/MultiplayerMenuState.hpp"
#include "states/PlayState.hpp"
#include "states/NetworkPlayState.hpp"
#include "core/Game.hpp"
#include "managers/StateManager.hpp"
#include "managers/NetworkManager.hpp"
#include "managers/MusicManager.hpp"
#include <components/UIElement.hpp>
#include <components/UIText.hpp>
#include <components/UIButton.hpp>
#include <components/UIPanel.hpp>
#include <components/UIInputField.hpp>
#include <components/Tag.hpp>
#include <scripting/LuaState.hpp>
#include <thread>
#include <chrono>
#include <SFML/Graphics.hpp>
#include "core/Logger.hpp"

MultiplayerMenuState::MultiplayerMenuState(Game* game)
{
    game_ = game;
}

void MultiplayerMenuState::onEnter()
{
    LOG_INFO("MULTIPLAYER", " Entering multiplayer menu");
    
    // Load ip/port from game_config.lua (single source of truth)
    loadNetworkConfig();

    // Setup network callback for game start
    auto networkMgr = game_->getNetworkManager();
    if (networkMgr) {
        networkMgr->setGameStartCallback([this]() {
            LOG_INFO("MULTIPLAYER", " GAME_START received from server!");
            shouldStartGame_ = true;  // Flag will be checked in update() for safe transition
        });
    }
    
    createMainMenu();

    // Play lobby/name entry music
    if (auto* music = game_->getMusicManager()) {
        music->play("assets/sounds/NAME ENTRY.ogg", true);
    }
}

void MultiplayerMenuState::onExit()
{
    LOG_INFO("MULTIPLAYER", "Exiting multiplayer menu");
    clearMenu();
}

void MultiplayerMenuState::createMainMenu()
{
    clearMenu();
    currentMode_ = MenuMode::MAIN;
    
    auto& lua = Scripting::LuaState::Instance().GetState();
    
    try {
        lua.script_file("assets/scripts/ui/menu_multiplayer.lua");
        sol::table menuConfig = lua["MultiplayerMenu"];
        
        auto coordinator = game_->getCoordinator();
        
        // Create background panel
        auto panelConfig = menuConfig["background"]["panel"];
        ECS::Entity panelEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIPanel"](panelEntity, 
                                  panelConfig["x"].get<float>(),
                                  panelConfig["y"].get<float>(),
                                  panelConfig["width"].get<float>(),
                                  panelConfig["height"].get<float>());
        menuEntities_.push_back(panelEntity);
        
        // Create title
        auto titleConfig = menuConfig["title"];
        ECS::Entity titleEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](titleEntity,
                                 titleConfig["x"].get<float>(),
                                 titleConfig["y"].get<float>(),
                                 titleConfig["text"].get<std::string>(),
                                 titleConfig["fontSize"].get<int>());
        menuEntities_.push_back(titleEntity);
        
        // Create buttons
        sol::table buttons = menuConfig["main_buttons"];
        for (size_t i = 1; i <= buttons.size(); ++i) {
            sol::table btn = buttons[i];
            
            ECS::Entity btnEntity = coordinator->CreateEntity();
            lua["ECS"]["AddUIButton"](btnEntity,
                                       btn["x"].get<float>(),
                                       btn["y"].get<float>(),
                                       btn["width"].get<float>(),
                                       btn["height"].get<float>(),
                                       btn["text"].get<std::string>(),
                                       btn["callback"].get<std::string>());
            menuEntities_.push_back(btnEntity);
        }
        
        LOG_INFO("MULTIPLAYER", " Created " + std::to_string(menuEntities_.size()) + " UI entities");
        
    } catch (const sol::error& e) {
        LOG_ERROR("MULTIPLAYER", " Error loading UI: " + std::string(e.what()));
    }
}

void MultiplayerMenuState::createHostMenu()
{
    clearMenu();
    currentMode_ = MenuMode::HOST;
    
    auto& lua = Scripting::LuaState::Instance().GetState();
    
    try {
        lua.script_file("assets/scripts/ui/menu_host.lua");
        sol::table menuConfig = lua["HostMenu"];
        
        auto coordinator = game_->getCoordinator();
        
        // Create background panel
        auto panelConfig = menuConfig["background"]["panel"];
        ECS::Entity panelEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIPanel"](panelEntity, 
                                  panelConfig["x"].get<float>(),
                                  panelConfig["y"].get<float>(),
                                  panelConfig["width"].get<float>(),
                                  panelConfig["height"].get<float>());
        menuEntities_.push_back(panelEntity);
        
        // Create title
        auto titleConfig = menuConfig["title"];
        ECS::Entity titleEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](titleEntity,
                                 titleConfig["x"].get<float>(),
                                 titleConfig["y"].get<float>(),
                                 titleConfig["text"].get<std::string>(),
                                 titleConfig["fontSize"].get<int>());
        menuEntities_.push_back(titleEntity);
        
        // Create labels
        auto roomLabel = menuConfig["room_label"];
        ECS::Entity roomLabelEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](roomLabelEntity,
                                 roomLabel["x"].get<float>(),
                                 roomLabel["y"].get<float>(),
                                 roomLabel["text"].get<std::string>(),
                                 roomLabel["fontSize"].get<int>());
        menuEntities_.push_back(roomLabelEntity);
        
        auto playersLabel = menuConfig["players_label"];
        ECS::Entity playersLabelEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](playersLabelEntity,
                                 playersLabel["x"].get<float>(),
                                 playersLabel["y"].get<float>(),
                                 playersLabel["text"].get<std::string>(),
                                 playersLabel["fontSize"].get<int>());
        menuEntities_.push_back(playersLabelEntity);
        
        // Create input fields
        auto roomInput = menuConfig["room_input"];
        ECS::Entity roomInputEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIInputField"](roomInputEntity,
                                       roomInput["x"].get<float>(),
                                       roomInput["y"].get<float>(),
                                       roomInput["width"].get<float>(),
                                       roomInput["height"].get<float>(),
                                       roomInput["placeholder"].get<std::string>(),
                                       roomInput["maxLength"].get<int>());
        menuEntities_.push_back(roomInputEntity);
        
        auto playersInput = menuConfig["players_input"];
        ECS::Entity playersInputEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIInputField"](playersInputEntity,
                                       playersInput["x"].get<float>(),
                                       playersInput["y"].get<float>(),
                                       playersInput["width"].get<float>(),
                                       playersInput["height"].get<float>(),
                                       playersInput["placeholder"].get<std::string>(),
                                       playersInput["maxLength"].get<int>());
        menuEntities_.push_back(playersInputEntity);
        
        // Create buttons
        sol::table buttons = menuConfig["buttons"];
        for (size_t i = 1; i <= buttons.size(); ++i) {
            sol::table btn = buttons[i];
            
            ECS::Entity btnEntity = coordinator->CreateEntity();
            lua["ECS"]["AddUIButton"](btnEntity,
                                       btn["x"].get<float>(),
                                       btn["y"].get<float>(),
                                       btn["width"].get<float>(),
                                       btn["height"].get<float>(),
                                       btn["text"].get<std::string>(),
                                       btn["callback"].get<std::string>());
            menuEntities_.push_back(btnEntity);
        }
        
        LOG_INFO("MULTIPLAYER", " Host menu created");
        
    } catch (const sol::error& e) {
        LOG_ERROR("MULTIPLAYER", " Error loading host UI: " + std::string(e.what()));
    }
}

void MultiplayerMenuState::createJoinMenu(bool isRefresh)
{
    clearMenu();
    currentMode_ = MenuMode::JOIN;
    
    auto& lua = Scripting::LuaState::Instance().GetState();
    
    try {
        lua.script_file("assets/scripts/ui/menu_join.lua");
        sol::table menuConfig = lua["JoinMenu"];
        
        auto coordinator = game_->getCoordinator();
        
        // Create background panel
        auto panelConfig = menuConfig["background"]["panel"];
        ECS::Entity panelEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIPanel"](panelEntity, 
                                  panelConfig["x"].get<float>(),
                                  panelConfig["y"].get<float>(),
                                  panelConfig["width"].get<float>(),
                                  panelConfig["height"].get<float>());
        menuEntities_.push_back(panelEntity);
        
        // Create title
        auto titleConfig = menuConfig["title"];
        ECS::Entity titleEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](titleEntity,
                                 titleConfig["x"].get<float>(),
                                 titleConfig["y"].get<float>(),
                                 titleConfig["text"].get<std::string>(),
                                 titleConfig["fontSize"].get<int>());
        menuEntities_.push_back(titleEntity);
        
        // Create subtitle
        auto subtitleConfig = menuConfig["subtitle"];
        ECS::Entity subtitleEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](subtitleEntity,
                                 subtitleConfig["x"].get<float>(),
                                 subtitleConfig["y"].get<float>(),
                                 subtitleConfig["text"].get<std::string>(),
                                 subtitleConfig["fontSize"].get<int>());
        menuEntities_.push_back(subtitleEntity);
        
        // Connect to server if not already connected
        auto networkMgr = game_->getNetworkManager();
        
        if (!networkMgr) {
            LOG_ERROR("MULTIPLAYER", " NetworkManager is null!");
            return;
        }
        
        // IMPORTANT: Only connect if NOT already connected or connecting
        // Calling connectToServer() while connected will disconnect first!
        if (!networkMgr->isConnected() && !isConnecting_) {
            LOG_INFO("MULTIPLAYER", "Auto-connecting to server...");
            isConnecting_ = true;
            networkMgr->connectToServer(serverAddress_, serverPort_, playerName_);
            isConnecting_ = false;
            
            // Wait a bit for connection to establish
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } else {
            LOG_INFO("MULTIPLAYER", "Already connected/connecting to server");
        }
        
        // Request fresh room list (non-blocking) - but only if this is the initial load
        // not a refresh (refresh already has the data we need)
        size_t roomCount = 0;
        if (networkMgr->isConnected()) {
            // Only request new room list on initial load, not on refresh
            if (!isRefresh) {
                LOG_INFO("MULTIPLAYER", "Requesting room list (non-blocking)...");
                networkMgr->requestRoomList();
                waitingForRoomList_ = true;
                lastRoomListVersion_ = networkMgr->getRoomListVersion();  // Track current version before response arrives
            } else {
                LOG_INFO("MULTIPLAYER", "Refresh mode - using cached room list");
            }
            
            // Use cached list for now, will auto-refresh when response arrives
            const auto& rooms = networkMgr->getRoomList();
            roomCount = rooms.size();
            lastRoomCount_ = roomCount;
            
            LOG_INFO("MULTIPLAYER", "Creating menu with " + std::to_string(roomCount) + " cached rooms");
            
            // Create room list items
            auto roomListConfig = menuConfig["room_list"];
            float startY = roomListConfig["y"].get<float>();
            float itemHeight = roomListConfig["item_height"].get<float>();
            
            LOG_INFO("MULTIPLAYER", "Creating " + std::to_string(rooms.size()) + " room buttons...");
            
            for (size_t i = 0; i < rooms.size(); ++i) {
                const auto& room = rooms[i];
                float yPos = startY + i * itemHeight;
                
                // Room button
                std::string roomText = std::string(room.roomName) + " (" + 
                                      std::to_string(room.currentPlayers) + "/" + 
                                      std::to_string(room.maxPlayers) + ")";
                if (room.inGame) {
                    roomText += " [IN GAME]";
                }
                
                LOG_INFO("MULTIPLAYER", "Creating room button: " + roomText + " (ID: " + std::to_string(room.roomId) + ")");
                
                ECS::Entity roomEntity = coordinator->CreateEntity();
                lua["ECS"]["AddUIButton"](roomEntity,
                                           450.0f,
                                           yPos,
                                           1020.0f,
                                           70.0f,
                                           roomText,
                                           "on_room_select");
                
                // Instead of adding another Tag (which overwrites), modify the existing one
                // AddUIButton already adds Tag{"button"}, so we update it to include room ID
                if (coordinator->HasComponent<Tag>(roomEntity)) {
                    auto& tag = coordinator->GetComponent<Tag>(roomEntity);
                    tag.name = "room_" + std::to_string(room.roomId);
                }
                
                LOG_INFO("MULTIPLAYER", "Room button created, entity: " + std::to_string(roomEntity));
                
                menuEntities_.push_back(roomEntity);
            }
        } else {
            LOG_ERROR("MULTIPLAYER", " Not connected, cannot request room list");
        }
        
        // Create buttons
        sol::table buttons = menuConfig["buttons"];
        for (size_t i = 1; i <= buttons.size(); ++i) {
            sol::table btn = buttons[i];
            
            ECS::Entity btnEntity = coordinator->CreateEntity();
            lua["ECS"]["AddUIButton"](btnEntity,
                                       btn["x"].get<float>(),
                                       btn["y"].get<float>(),
                                       btn["width"].get<float>(),
                                       btn["height"].get<float>(),
                                       btn["text"].get<std::string>(),
                                       btn["callback"].get<std::string>());
            menuEntities_.push_back(btnEntity);
        }
        
        LOG_INFO("MULTIPLAYER", " Join menu created with " + std::to_string(roomCount) + " rooms");
        
    } catch (const sol::error& e) {
        LOG_ERROR("MULTIPLAYER", " Error loading join UI: " + std::string(e.what()));
    }
}

void MultiplayerMenuState::createLobbyMenu()
{
    // Preserve chat input text across lobby refreshes
    {
        auto coordinator = game_->getCoordinator();
        for (auto entity : menuEntities_) {
            if (coordinator->HasComponent<Components::UIInputField>(entity)) {
                auto& field = coordinator->GetComponent<Components::UIInputField>(entity);
                if (field.placeholder == "Type a message...") {
                    pendingChatText_ = field.text;
                    break;
                }
            }
        }
    }
    
    clearMenu();
    currentMode_ = MenuMode::LOBBY;
    // Note: isReady_ is preserved across refreshes to maintain state
    
    auto& lua = Scripting::LuaState::Instance().GetState();
    
    try {
        lua.script_file("assets/scripts/ui/menu_lobby.lua");
        sol::table menuConfig = lua["LobbyMenu"];
        
        auto coordinator = game_->getCoordinator();
        
        // Create background panel
        auto panelConfig = menuConfig["background"]["panel"];
        ECS::Entity panelEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIPanel"](panelEntity, 
                                  panelConfig["x"].get<float>(),
                                  panelConfig["y"].get<float>(),
                                  panelConfig["width"].get<float>(),
                                  panelConfig["height"].get<float>());
        menuEntities_.push_back(panelEntity);
        
        // Create title
        auto titleConfig = menuConfig["title"];
        ECS::Entity titleEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](titleEntity,
                                 titleConfig["x"].get<float>(),
                                 titleConfig["y"].get<float>(),
                                 titleConfig["text"].get<std::string>(),
                                 titleConfig["fontSize"].get<int>());
        menuEntities_.push_back(titleEntity);
        
        // Create room info text
        auto roomInfoConfig = menuConfig["room_info"];
        ECS::Entity roomInfoEntity = coordinator->CreateEntity();
        
        // Get room name from NetworkManager
        auto networkMgr = game_->getNetworkManager();
        std::string roomName = "Unknown Room";
        uint8_t maxPlayers = 4;
        if (networkMgr && networkMgr->getCurrentRoomId() > 0) {
            // Use stored room name (set when creating or joining)
            if (!networkMgr->getCurrentRoomName().empty()) {
                roomName = networkMgr->getCurrentRoomName();
                maxPlayers = networkMgr->getCurrentMaxPlayers();
            } else {
                // Fallback: search in room list
                const auto& rooms = networkMgr->getRoomList();
                for (const auto& room : rooms) {
                    if (room.roomId == networkMgr->getCurrentRoomId()) {
                        roomName = std::string(room.roomName);
                        maxPlayers = room.maxPlayers;
                        break;
                    }
                }
            }
        }
        
        std::string roomDisplayText = roomName + " (max " + std::to_string(maxPlayers) + " players)";
        
        lua["ECS"]["AddUIText"](roomInfoEntity,
                                 roomInfoConfig["x"].get<float>(),
                                 roomInfoConfig["y"].get<float>(),
                                 roomDisplayText,
                                 roomInfoConfig["fontSize"].get<int>());
        menuEntities_.push_back(roomInfoEntity);
        
        // Create player list
        auto playerListConfig = menuConfig["player_list"];
        
        // Player list title
        ECS::Entity listTitleEntity = coordinator->CreateEntity();
        lua["ECS"]["AddUIText"](listTitleEntity,
                                 playerListConfig["x"].get<float>(),
                                 playerListConfig["y"].get<float>(),
                                 playerListConfig["title"].get<std::string>(),
                                 playerListConfig["title_fontSize"].get<int>());
        menuEntities_.push_back(listTitleEntity);
        
        // TODO Phase 5.3: Get real player list from NetworkManager
        std::vector<std::string> players;
        std::vector<bool> readyStates;
        
        if (networkMgr && networkMgr->getCurrentRoomId() != 0) {
            // Use real player data from NetworkManager
            const auto& roomPlayers = networkMgr->getRoomPlayers();
            uint32_t localPlayerId = networkMgr->getLocalPlayerId();
            
            for (const auto& player : roomPlayers) {
                std::string displayName = player.playerName;
                // Mark "You" for local player using the player ID assigned by server
                if (player.playerId == localPlayerId) {
                    displayName += " (You)";
                }
                players.push_back(displayName);
                readyStates.push_back(player.isReady);
            }
        }
        
        // Fallback: If no players yet, show local player only
        if (players.empty()) {
            players.push_back(playerName_ + " (You)");
            readyStates.push_back(isReady_);
        }
        
        float startY = playerListConfig["y"].get<float>() + 40.0f;
        float itemHeight = playerListConfig["item_height"].get<float>();
        
        for (size_t i = 0; i < players.size(); ++i) {
            float yPos = startY + i * itemHeight;
            
            // Player name text
            std::string playerText = players[i];
            if (readyStates[i]) {
                playerText += " [READY]";
            } else {
                playerText += " - Not Ready";
            }
            
            ECS::Entity playerEntity = coordinator->CreateEntity();
            lua["ECS"]["AddUIText"](playerEntity,
                                     playerListConfig["x"].get<float>() + 20.0f,
                                     yPos,
                                     playerText,
                                     22);
            menuEntities_.push_back(playerEntity);
        }
        
        // === Chat area ===
        if (menuConfig["chat"].valid()) {
            auto chatConfig = menuConfig["chat"];
            float chatX = chatConfig["x"].get<float>();
            float chatY = chatConfig["y"].get<float>();
            float lineHeight = chatConfig["line_height"].get<float>();
            int msgFontSize = chatConfig["message_fontSize"].get<int>();
            int maxVisible = chatConfig["max_visible"].get<int>();
            
            // Chat title
            ECS::Entity chatTitleEntity = coordinator->CreateEntity();
            lua["ECS"]["AddUIText"](chatTitleEntity,
                                     chatX,
                                     chatY,
                                     chatConfig["title"].get<std::string>(),
                                     chatConfig["title_fontSize"].get<int>());
            menuEntities_.push_back(chatTitleEntity);
            
            // Display chat messages
            if (networkMgr) {
                const auto& messages = networkMgr->getChatMessages();
                size_t startIdx = 0;
                if (messages.size() > static_cast<size_t>(maxVisible)) {
                    startIdx = messages.size() - maxVisible;
                }
                
                float msgY = chatY + 30.0f;
                for (size_t i = startIdx; i < messages.size(); ++i) {
                    std::string line = messages[i].senderName + ": " + messages[i].message;
                    
                    ECS::Entity msgEntity = coordinator->CreateEntity();
                    lua["ECS"]["AddUIText"](msgEntity,
                                             chatX + 10.0f,
                                             msgY,
                                             line,
                                             msgFontSize);
                    menuEntities_.push_back(msgEntity);
                    msgY += lineHeight;
                }
            }
            
            // Chat input field
            float inputY = chatConfig["input_y"].get<float>();
            float inputWidth = chatConfig["input_width"].get<float>();
            float inputHeight = chatConfig["input_height"].get<float>();
            
            ECS::Entity chatInputEntity = coordinator->CreateEntity();
            lua["ECS"]["AddUIInputField"](chatInputEntity,
                                           chatX,
                                           inputY,
                                           inputWidth,
                                           inputHeight,
                                           "Type a message...",
                                           128);
            // Restore pending chat text from before refresh
            if (!pendingChatText_.empty()) {
                auto& chatField = coordinator->GetComponent<Components::UIInputField>(chatInputEntity);
                chatField.text = pendingChatText_;
                chatField.cursorPosition = pendingChatText_.size();
                chatField.isFocused = true;
                pendingChatText_.clear();
            }
            menuEntities_.push_back(chatInputEntity);
        }
        
        // Create buttons
        sol::table buttons = menuConfig["buttons"];
        bool isHost = (networkMgr && networkMgr->isHosting());
        
        for (size_t i = 1; i <= buttons.size(); ++i) {
            sol::table btn = buttons[i];
            std::string btnText = btn["text"].get<std::string>();
            
            // Skip "Start Game" button if not host
            if (btnText == "Start Game" && !isHost) {
                LOG_INFO("MULTIPLAYER", "Hiding Start Game button (not host)");
                continue;
            }
            
            // Grey out "Start Game" if not all players are ready
            if (btnText == "Start Game" && networkMgr) {
                const auto& roomPlayers = networkMgr->getRoomPlayers();
                bool allReady = !roomPlayers.empty();
                for (const auto& p : roomPlayers) {
                    if (!p.isReady) { allReady = false; break; }
                }
                if (!allReady) {
                    btnText = "Waiting for players...";
                }
            }
            
            // Update Ready button text based on current state
            if (btnText == "Ready") {
                btnText = isReady_ ? "Not Ready" : "Ready";
            }
            
            ECS::Entity btnEntity = coordinator->CreateEntity();
            lua["ECS"]["AddUIButton"](btnEntity,
                                       btn["x"].get<float>(),
                                       btn["y"].get<float>(),
                                       btn["width"].get<float>(),
                                       btn["height"].get<float>(),
                                       btnText,
                                       btn["callback"].get<std::string>());
            menuEntities_.push_back(btnEntity);
        }
        
        // Initialize tracking for auto-refresh
        lastPlayerCount_ = players.size();
        lastReadyStates_.clear();
        for (bool ready : readyStates) {
            lastReadyStates_.push_back(ready);
        }
        if (networkMgr) {
            lastChatVersion_ = networkMgr->getChatVersion();
        }
        
        LOG_INFO("MULTIPLAYER", " Lobby menu created with " + std::to_string(players.size()) + " players");
        
    } catch (const sol::error& e) {
        LOG_ERROR("MULTIPLAYER", " Error loading lobby UI: " + std::string(e.what()));
    }
}

void MultiplayerMenuState::clearMenu()
{
    auto coordinator = game_->getCoordinator();
    for (auto entity : menuEntities_) {
        coordinator->DestroyEntity(entity);
    }
    menuEntities_.clear();
    hoveredButton_ = 0;
}

void MultiplayerMenuState::handleEvent(const eng::engine::InputEvent& event)
{
    auto coordinator = game_->getCoordinator();
    if (!coordinator) return;
    
    // Set flag to prevent menu refresh during event handling
    isHandlingEvent_ = true;
    
    // Handle mouse movement for hover
    if (event.type == eng::engine::EventType::MouseMoved)
    {
        float mouseX = static_cast<float>(event.mouseMove.x);
        float mouseY = static_cast<float>(event.mouseMove.y);
        
        // Reset all buttons to Normal
        for (auto entity : menuEntities_)
        {
            if (coordinator->HasComponent<Components::UIButton>(entity))
            {
                auto& button = coordinator->GetComponent<Components::UIButton>(entity);
                button.state = Components::UIButton::State::Normal;
            }
        }
        
        // Check hover
        for (auto entity : menuEntities_)
        {
            if (!coordinator->HasComponent<Components::UIButton>(entity)) continue;
            if (!coordinator->HasComponent<Components::UIElement>(entity)) continue;
            
            auto& uiElem = coordinator->GetComponent<Components::UIElement>(entity);
            auto& button = coordinator->GetComponent<Components::UIButton>(entity);
            
            if (mouseX >= uiElem.x && mouseX <= uiElem.x + uiElem.width &&
                mouseY >= uiElem.y && mouseY <= uiElem.y + uiElem.height)
            {
                button.state = Components::UIButton::State::Hovered;
                hoveredButton_ = entity;
                break;
            }
        }
    }
    
    // Handle mouse clicks
    if (event.type == eng::engine::EventType::MouseButtonPressed)
    {
        if (event.mouseButton.button == 0)
        {
            float mouseX = static_cast<float>(event.mouseButton.x);
            float mouseY = static_cast<float>(event.mouseButton.y);
            
            // Check input field clicks (for focus)
            bool inputFieldClicked = false;
            for (auto entity : menuEntities_)
            {
                if (!coordinator->HasComponent<Components::UIInputField>(entity)) continue;
                if (!coordinator->HasComponent<Components::UIElement>(entity)) continue;
                
                auto& uiElem = coordinator->GetComponent<Components::UIElement>(entity);
                auto& inputField = coordinator->GetComponent<Components::UIInputField>(entity);
                
                if (mouseX >= uiElem.x && mouseX <= uiElem.x + uiElem.width &&
                    mouseY >= uiElem.y && mouseY <= uiElem.y + uiElem.height)
                {
                    // Focus this field
                    inputField.isFocused = true;
                    inputField.cursorPosition = inputField.text.length();
                    inputFieldClicked = true;
                    LOG_INFO("MULTIPLAYER", "Input field focused");
                }
                else
                {
                    // Unfocus other fields
                    inputField.isFocused = false;
                }
            }
            
            // If no input field clicked, check buttons
            if (!inputFieldClicked)
            {
                for (auto entity : menuEntities_)
                {
                if (!coordinator->HasComponent<Components::UIButton>(entity)) continue;
                if (!coordinator->HasComponent<Components::UIElement>(entity)) continue;
                
                auto& uiElem = coordinator->GetComponent<Components::UIElement>(entity);
                auto& button = coordinator->GetComponent<Components::UIButton>(entity);
                
                if (mouseX >= uiElem.x && mouseX <= uiElem.x + uiElem.width &&
                    mouseY >= uiElem.y && mouseY <= uiElem.y + uiElem.height)
                {
                    LOG_INFO("MULTIPLAYER", "Button clicked: " + button.text);
                    
                    // Main menu buttons
                    if (button.text == "Back")
                    {
                        if (currentMode_ == MenuMode::MAIN) {
                            game_->getStateManager()->popState();
                        } else {
                            createMainMenu(); // Return to multiplayer main menu
                        }
                    }
                    else if (button.text == "Host Game")
                    {
                        createHostMenu();
                    }
                    else if (button.text == "Join Game")
                    {
                        createJoinMenu();
                    }
                    
                    // Host menu buttons
                    else if (button.text == "Start Server")
                    {
                        auto networkMgr = game_->getNetworkManager();
                        if (networkMgr) {
                            // Read values from input fields
                            std::string roomName = playerName_ + "'s Room";
                            uint8_t maxPlayers = 4; // Default
                            
                            for (auto inputEntity : menuEntities_) {
                                if (!coordinator->HasComponent<Components::UIInputField>(inputEntity)) continue;
                                auto& field = coordinator->GetComponent<Components::UIInputField>(inputEntity);
                                
                                if (field.placeholder == "Enter room name..." && !field.text.empty()) {
                                    roomName = field.text;
                                } else if (field.placeholder == "4" && !field.text.empty()) {
                                    try {
                                        int val = std::stoi(field.text);
                                        if (val >= 2 && val <= 4) maxPlayers = static_cast<uint8_t>(val);
                                    } catch (...) {}
                                }
                            }
                            
                            networkMgr->startServer(serverPort_, maxPlayers);
                            networkMgr->createRoom(roomName, maxPlayers);
                            LOG_INFO("MULTIPLAYER", " Server started! Room: '" + roomName
                                      + "', Max players: " + std::to_string((int)maxPlayers) + ", Port: " + std::to_string(serverPort_));
                            createLobbyMenu();
                        }
                    }
                    else if (button.text == "Cancel")
                    {
                        createMainMenu();
                    }
                    
                    // Join menu buttons
                    else if (button.text == "Refresh")
                    {
                        auto networkMgr = game_->getNetworkManager();
                        if (networkMgr) {
                            LOG_INFO("MULTIPLAYER", "Refreshing room list...");
                            networkMgr->requestRoomList();
                            waitingForRoomList_ = true;
                            lastRoomListVersion_ = networkMgr->getRoomListVersion();  // Track current version
                            LOG_INFO("MULTIPLAYER", "Room list request sent. Room list will update automatically.");
                            // Don't recreate the menu - the list will update on next frame when response arrives
                        }
                    }
                    
                    // Room selection (any other button in Join menu)
                    else if (currentMode_ == MenuMode::JOIN && button.text.find("(") != std::string::npos)
                    {
                        LOG_INFO("MULTIPLAYER", "Selected room: " + button.text);
                        
                        // Extract room ID from Tag component
                        uint32_t roomId = 0;
                        if (coordinator->HasComponent<Tag>(entity)) {
                            auto& tag = coordinator->GetComponent<Tag>(entity);
                            // Tag format: "room_123"
                            if (tag.name.find("room_") == 0) {
                                try {
                                    roomId = std::stoul(tag.name.substr(5));
                                } catch (...) {
                                    LOG_ERROR("MULTIPLAYER", "Failed to extract room ID from tag: " + tag.name);
                                }
                            }
                        }
                        
                        if (roomId == 0) {
                            LOG_ERROR("MULTIPLAYER", "Invalid room ID");
                            break;
                        }
                        
                        auto networkMgr = game_->getNetworkManager();
                        if (networkMgr) {
                            // Already connected in createJoinMenu(), just join the room
                            LOG_INFO("MULTIPLAYER", "Joining room " + std::to_string(roomId));
                            networkMgr->joinRoom(roomId);
                            
                            // Process packets while waiting for ROOM_JOINED response (max 300ms)
                            auto startTime = std::chrono::steady_clock::now();
                            bool joined = false;
                            while (std::chrono::steady_clock::now() - startTime < std::chrono::milliseconds(300)) {
                                networkMgr->update();  // Process incoming packets
                                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                                // Check if we got room players (means ROOM_UPDATE was received)
                                if (!networkMgr->getRoomPlayers().empty()) {
                                    joined = true;
                                    break;
                                }
                            }
                            
                            if (joined) {
                                LOG_INFO("MULTIPLAYER", " Successfully joined room " + std::to_string(roomId));
                                createLobbyMenu();
                            } else {
                                LOG_ERROR("MULTIPLAYER", " Failed to join room (timeout)");
                            }
                        }
                    }
                    
                    // Lobby menu buttons
                    else if (button.text == "Ready" || button.text == "Not Ready")
                    {
                        auto networkMgr = game_->getNetworkManager();
                        if (networkMgr) {
                            // Toggle ready state
                            isReady_ = !isReady_;
                            networkMgr->setReady(isReady_);
                            LOG_INFO("MULTIPLAYER", std::string(" Ready state: ") + (isReady_ ? "READY" : "NOT READY"));
                            
                            // Don't refresh immediately - wait for server response via ROOM_UPDATE
                            // The update() loop will detect the change and refresh automatically
                        }
                    }
                    else if (button.text == "Start Game")
                    {
                        auto networkMgr = game_->getNetworkManager();
                        if (networkMgr && networkMgr->isHosting()) {
                            // Check that all players in the room are ready
                            const auto& roomPlayers = networkMgr->getRoomPlayers();
                            bool allReady = !roomPlayers.empty();
                            for (const auto& p : roomPlayers) {
                                if (!p.isReady) {
                                    allReady = false;
                                    break;
                                }
                            }
                            if (allReady) {
                                LOG_INFO("MULTIPLAYER", " All players ready — starting game!");
                                networkMgr->startGame();
                                LOG_INFO("MULTIPLAYER", " Host sent GAME_START packet");
                            } else {
                                LOG_INFO("MULTIPLAYER", " Cannot start: not all players are ready");
                            }
                        }
                    }
                    else if (button.text == "Leave Room")
                    {
                        LOG_INFO("MULTIPLAYER", "Leaving room...");
                        auto networkMgr = game_->getNetworkManager();
                        if (networkMgr) {
                            networkMgr->leaveRoom();
                            // Don't disconnect - stay connected to browse rooms
                        }
                        isReady_ = false;  // Reset ready state
                        createMainMenu(); // Return to multiplayer main menu
                    }
                    
                    break;
                }
            }
            } // End of !inputFieldClicked
        }
    }
    
    // Handle text input for focused input fields
    if (event.type == eng::engine::EventType::TextEntered)
    {
        for (auto entity : menuEntities_)
        {
            if (!coordinator->HasComponent<Components::UIInputField>(entity)) continue;
            
            auto& inputField = coordinator->GetComponent<Components::UIInputField>(entity);
            
            if (inputField.isFocused)
            {
                char c = static_cast<char>(event.text.unicode);
                
                // Handle Enter key - send chat message if in lobby
                if (c == 13 && currentMode_ == MenuMode::LOBBY && inputField.placeholder == "Type a message...")
                {
                    if (!inputField.text.empty()) {
                        auto networkMgr = game_->getNetworkManager();
                        if (networkMgr) {
                            networkMgr->sendChatMessage(inputField.text);
                        }
                        inputField.text.clear();
                        inputField.cursorPosition = 0;
                    }
                }
                // Handle backspace
                else if (c == 8 && !inputField.text.empty() && inputField.cursorPosition > 0)
                {
                    inputField.text.erase(inputField.cursorPosition - 1, 1);
                    inputField.cursorPosition--;
                }
                // Handle printable characters
                else if (c >= 32 && c < 127 && inputField.text.length() < inputField.maxLength)
                {
                    inputField.text.insert(inputField.cursorPosition, 1, c);
                    inputField.cursorPosition++;
                }
                
                break;
            }
        }
    }
    
    // ESC to go back
    if (event.type == eng::engine::EventType::KeyPressed)
    {
        if (event.key.code == eng::engine::Key::Escape)
        {
            LOG_INFO("MULTIPLAYER", "ESC pressed - going back");
            if (currentMode_ == MenuMode::MAIN) {
                game_->getStateManager()->popState();
            } else {
                createMainMenu();
            }
        }
    }
    
    // Clear flag at end of event handling
    isHandlingEvent_ = false;
}

void MultiplayerMenuState::update(float deltaTime)
{
    (void)deltaTime;
    
    // Check if we should transition to NetworkPlayState (GAME_START received)
    if (shouldStartGame_) {
        shouldStartGame_ = false;
        LOG_INFO("MULTIPLAYER", " Transitioning to NetworkPlayState (multiplayer)...");
        game_->getStateManager()->pushState(std::make_unique<NetworkPlayState>(game_));
        return;
    }
    
    // Don't refresh during event handling
    if (isHandlingEvent_) {
        return;
    }
    
    // Handle deferred menu refresh (safe - at start of frame, not during event handling)
    if (needsMenuRefresh_) {
        needsMenuRefresh_ = false;
        if (currentMode_ == MenuMode::JOIN) {
            createJoinMenu(true);  // true = refresh mode, don't re-request room list
        } else if (currentMode_ == MenuMode::LOBBY) {
            createLobbyMenu();  // Refresh lobby
        }
        return;  // Don't do anything else this frame
    }
    
    // Check if room list changed and we need to refresh (JOIN menu)
    if (currentMode_ == MenuMode::JOIN && waitingForRoomList_) {
        auto networkMgr = game_->getNetworkManager();
        if (networkMgr) {
            uint32_t currentVersion = networkMgr->getRoomListVersion();
            if (currentVersion != lastRoomListVersion_) {
                const auto& rooms = networkMgr->getRoomList();
                LOG_INFO("MULTIPLAYER", "Room list updated (version " + std::to_string(lastRoomListVersion_)
                          + " -> " + std::to_string(currentVersion) + ", " + std::to_string(rooms.size()) + " rooms), will refresh next frame...");
                lastRoomListVersion_ = currentVersion;
                lastRoomCount_ = rooms.size();
                waitingForRoomList_ = false;
                needsMenuRefresh_ = true;  // Defer refresh to next frame for safety
            }
        }
    }
    
    // Auto-refresh lobby when player list or ready states change
    if (currentMode_ == MenuMode::LOBBY) {
        auto networkMgr = game_->getNetworkManager();
        if (networkMgr) {
            const auto& players = networkMgr->getRoomPlayers();
            
            // Check if player count changed
            bool needsRefresh = (players.size() != lastPlayerCount_);
            
            // Check if any ready state changed
            if (!needsRefresh && players.size() == lastReadyStates_.size()) {
                for (size_t i = 0; i < players.size(); ++i) {
                    if (players[i].isReady != lastReadyStates_[i]) {
                        LOG_INFO("MULTIPLAYER", "Ready state changed for player " + std::to_string(i)
                                  + ": " + std::to_string(lastReadyStates_[i]) + " -> " + std::to_string(players[i].isReady));
                        needsRefresh = true;
                        break;
                    }
                }
            } else if (players.size() != lastReadyStates_.size()) {
                needsRefresh = true;
            }
            
            if (needsRefresh) {
                LOG_INFO("MULTIPLAYER", "Lobby updated (players: " + std::to_string(lastPlayerCount_)
                          + " -> " + std::to_string(players.size()) + "), refreshing...");
                lastPlayerCount_ = players.size();
                lastReadyStates_.clear();
                for (const auto& p : players) {
                    lastReadyStates_.push_back(p.isReady);
                }
                needsMenuRefresh_ = true;
            }
            
            // Auto-refresh when new chat messages arrive
            uint32_t currentChatVer = networkMgr->getChatVersion();
            if (currentChatVer != lastChatVersion_) {
                lastChatVersion_ = currentChatVer;
                needsMenuRefresh_ = true;
            }
        }
    }
}

void MultiplayerMenuState::render()
{
    auto window = game_->getWindow();
    auto coordinator = game_->getCoordinator();
    if (!window || !coordinator) return;
    
    auto& sfmlWindow = window->getSFMLWindow();
    
    // Render all UI entities (same pattern as other states)
    for (auto entity : menuEntities_)
    {
        if (!coordinator->HasComponent<Components::UIElement>(entity)) continue;
        
        auto& uiElem = coordinator->GetComponent<Components::UIElement>(entity);
        if (!uiElem.visible) continue;
        
        // Render panels
        if (coordinator->HasComponent<Components::UIPanel>(entity))
        {
            auto& panel = coordinator->GetComponent<Components::UIPanel>(entity);
            
            sf::RectangleShape rect(sf::Vector2f(uiElem.width, uiElem.height));
            rect.setPosition(uiElem.x, uiElem.y);
            
            sf::Color bgColor(
                (panel.backgroundColor >> 24) & 0xFF,
                (panel.backgroundColor >> 16) & 0xFF,
                (panel.backgroundColor >> 8) & 0xFF,
                panel.backgroundColor & 0xFF
            );
            rect.setFillColor(bgColor);
            
            if (panel.borderThickness > 0) {
                sf::Color borderCol(
                    (panel.borderColor >> 24) & 0xFF,
                    (panel.borderColor >> 16) & 0xFF,
                    (panel.borderColor >> 8) & 0xFF,
                    panel.borderColor & 0xFF
                );
                rect.setOutlineColor(borderCol);
                rect.setOutlineThickness(panel.borderThickness);
            }
            
            sfmlWindow.draw(rect);
        }
        
        // Render buttons
        if (coordinator->HasComponent<Components::UIButton>(entity))
        {
            auto& button = coordinator->GetComponent<Components::UIButton>(entity);
            
            sf::RectangleShape rect(sf::Vector2f(uiElem.width, uiElem.height));
            rect.setPosition(uiElem.x, uiElem.y);
            
            uint32_t colorVal = button.normalColor;
            if (button.state == Components::UIButton::State::Hovered) {
                colorVal = button.hoverColor;
            } else if (button.state == Components::UIButton::State::Pressed) {
                colorVal = button.pressedColor;
            }
            
            sf::Color btnColor(
                (colorVal >> 24) & 0xFF,
                (colorVal >> 16) & 0xFF,
                (colorVal >> 8) & 0xFF,
                colorVal & 0xFF
            );
            rect.setFillColor(btnColor);
            rect.setOutlineColor(sf::Color::White);
            rect.setOutlineThickness(2.0f);
            
            sfmlWindow.draw(rect);
            
            if (!button.text.empty())
            {
                static sf::Font font;
                static bool fontLoaded = false;
                if (!fontLoaded) {
                    font.loadFromFile("assets/fonts/main_font.ttf");
                    fontLoaded = true;
                }
                
                sf::Text text;
                text.setFont(font);
                text.setString(button.text);
                text.setCharacterSize(24);
                text.setFillColor(sf::Color::White);
                
                sf::FloatRect textBounds = text.getLocalBounds();
                text.setPosition(
                    uiElem.x + (uiElem.width - textBounds.width) / 2.0f - textBounds.left,
                    uiElem.y + (uiElem.height - textBounds.height) / 2.0f - textBounds.top
                );
                
                sfmlWindow.draw(text);
            }
        }
        
        // Render text
        if (coordinator->HasComponent<Components::UIText>(entity))
        {
            auto& uiText = coordinator->GetComponent<Components::UIText>(entity);
            
            static sf::Font font;
            static bool fontLoaded = false;
            if (!fontLoaded) {
                font.loadFromFile("assets/fonts/main_font.ttf");
                fontLoaded = true;
            }
            
            sf::Text text;
            text.setFont(font);
            text.setString(uiText.content);
            text.setCharacterSize(uiText.fontSize);
            
            sf::Color textColor(
                (uiText.color >> 24) & 0xFF,
                (uiText.color >> 16) & 0xFF,
                (uiText.color >> 8) & 0xFF,
                uiText.color & 0xFF
            );
            text.setFillColor(textColor);
            
            sf::FloatRect textBounds = text.getLocalBounds();
            float xPos = uiElem.x;
            if (uiText.alignment == Components::UIText::Alignment::Center) {
                xPos -= textBounds.width / 2.0f;
            } else if (uiText.alignment == Components::UIText::Alignment::Right) {
                xPos -= textBounds.width;
            }
            
            text.setPosition(xPos - textBounds.left, uiElem.y - textBounds.top);
            
            sfmlWindow.draw(text);
        }
        
        // Render input fields
        if (coordinator->HasComponent<Components::UIInputField>(entity))
        {
            auto& inputField = coordinator->GetComponent<Components::UIInputField>(entity);
            
            // Background
            sf::RectangleShape rect(sf::Vector2f(uiElem.width, uiElem.height));
            rect.setPosition(uiElem.x, uiElem.y);
            
            sf::Color bgColor(
                (inputField.backgroundColor >> 24) & 0xFF,
                (inputField.backgroundColor >> 16) & 0xFF,
                (inputField.backgroundColor >> 8) & 0xFF,
                inputField.backgroundColor & 0xFF
            );
            rect.setFillColor(bgColor);
            
            // Border (focused or normal)
            uint32_t borderColorVal = inputField.isFocused ? inputField.focusBorderColor : inputField.borderColor;
            sf::Color borderColor(
                (borderColorVal >> 24) & 0xFF,
                (borderColorVal >> 16) & 0xFF,
                (borderColorVal >> 8) & 0xFF,
                borderColorVal & 0xFF
            );
            rect.setOutlineColor(borderColor);
            rect.setOutlineThickness(inputField.borderThickness);
            
            sfmlWindow.draw(rect);
            
            // Text or placeholder
            static sf::Font font;
            static bool fontLoaded = false;
            if (!fontLoaded) {
                font.loadFromFile("assets/fonts/main_font.ttf");
                fontLoaded = true;
            }
            
            sf::Text text;
            text.setFont(font);
            text.setCharacterSize(20);
            
            if (inputField.text.empty()) {
                // Show placeholder
                text.setString(inputField.placeholder);
                sf::Color placeholderCol(
                    (inputField.placeholderColor >> 24) & 0xFF,
                    (inputField.placeholderColor >> 16) & 0xFF,
                    (inputField.placeholderColor >> 8) & 0xFF,
                    inputField.placeholderColor & 0xFF
                );
                text.setFillColor(placeholderCol);
            } else {
                // Show actual text
                text.setString(inputField.text);
                sf::Color textCol(
                    (inputField.textColor >> 24) & 0xFF,
                    (inputField.textColor >> 16) & 0xFF,
                    (inputField.textColor >> 8) & 0xFF,
                    inputField.textColor & 0xFF
                );
                text.setFillColor(textCol);
            }
            
            text.setPosition(uiElem.x + inputField.padding, 
                           uiElem.y + (uiElem.height - 20) / 2.0f);
            
            sfmlWindow.draw(text);
            
            // Cursor (if focused and visible)
            if (inputField.isFocused && inputField.cursorVisible) {
                sf::RectangleShape cursor(sf::Vector2f(2.0f, uiElem.height - inputField.padding * 2));
                float cursorX = uiElem.x + inputField.padding;
                if (!inputField.text.empty()) {
                    sf::Text tempText;
                    tempText.setFont(font);
                    tempText.setCharacterSize(20);
                    tempText.setString(inputField.text.substr(0, inputField.cursorPosition));
                    cursorX += tempText.getLocalBounds().width;
                }
                cursor.setPosition(cursorX, uiElem.y + inputField.padding);
                cursor.setFillColor(sf::Color::White);
                sfmlWindow.draw(cursor);
            }
        }
    }
}

void MultiplayerMenuState::loadNetworkConfig()
{
    try {
        auto& lua = Scripting::LuaState::Instance().GetState();
        lua.script_file("assets/scripts/config/game_config.lua");
        sol::table gameConfig = lua["Game"];
        if (gameConfig.valid()) {
            sol::optional<sol::table> netT = gameConfig["network"];
            if (netT) {
                serverAddress_ = netT.value().get_or<std::string>("server_ip", serverAddress_);
                serverPort_    = static_cast<uint16_t>(netT.value().get_or("server_port", (int)serverPort_));
            }
        }
        LOG_INFO("MULTIPLAYER", "Network config: " + serverAddress_ + ":" + std::to_string(serverPort_));
    } catch (const std::exception& e) {
        LOG_WARNING("MULTIPLAYER", std::string(" Could not load network config, using defaults: ") + e.what());
    }
}
