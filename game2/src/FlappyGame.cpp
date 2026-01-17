#include "FlappyGame.hpp"
#include <filesystem>
#include <fstream>
#include <components/UIText.hpp>
#include <components/UIButton.hpp>
#include <components/UIPanel.hpp>
#include <components/Velocity.hpp>

using namespace eng::engine::rendering::sfml;

// Use ECS components from engine (same as Lua bindings)
// Note: We use the eng::engine::ECS:: namespace for components that match Lua bindings
using eng::engine::ECS::Transform;
using eng::engine::ECS::Health;
using eng::engine::ECS::Damage;
// Note: Sprite, Tag, and Velocity have two versions:
// - eng::engine::ECS::Velocity/Sprite/Tag (for Lua)
// - ::Velocity/Sprite/Tag from components/ (for engine systems)
// We'll use namespace qualifiers to disambiguate

namespace FlappyBird {

std::string FlappyGame::ResolveAssetPath(const std::string& relativePath) {
    // If we already found the base path, use it
    if (!basePath.empty()) {
        return basePath + relativePath;
    }

    // List of possible base paths to check
    std::vector<std::string> basePaths = {
        "",              // Current directory
        "../../",        // Running from build/game2/
        "../../../",     // Running from deeper build directories
    };

    // Test file to check if we're in the right directory
    std::string testFile = "game2/assets/sprites/bird.png";

    for (const auto& base : basePaths) {
        std::string fullPath = base + testFile;
        std::ifstream file(fullPath);
        if (file.good()) {
            basePath = base;
            std::cout << "[FlappyGame] Base path resolved to: " << (base.empty() ? "(current dir)" : base) << std::endl;
            return basePath + relativePath;
        }
    }

    // Fallback: return the path as-is
    std::cerr << "[FlappyGame] Warning: Could not resolve base path, using relative path as-is" << std::endl;
    return relativePath;
}

void FlappyGame::RegisterEntity(ECS::Entity entity) {
    allEntities.push_back(entity);
}

void FlappyGame::DestroyEntityDeferred(ECS::Entity entity) {
    entitiesToDestroy.push_back(entity);
}

void FlappyGame::ProcessDestroyedEntities() {
    for (auto entity : entitiesToDestroy) {
        // Clean up sprite if exists (use ::Sprite from components/)
        if (gCoordinator.HasComponent<::Sprite>(entity)) {
            auto& sprite = gCoordinator.GetComponent<::Sprite>(entity);
            if (sprite.sprite) {
                // Remove from allSprites tracking (raw ptr sprites we created)
                auto it = std::find(allSprites.begin(), allSprites.end(), sprite.sprite);
                if (it != allSprites.end()) {
                    allSprites.erase(it);
                }
                sprite.sprite = nullptr;
            }
        }

        gCoordinator.DestroyEntity(entity);

        // Remove from allEntities
        allEntities.erase(std::remove(allEntities.begin(), allEntities.end(), entity), allEntities.end());
    }
    entitiesToDestroy.clear();
}

FlappyGame::FlappyGame() : serverAddress("127.0.0.1"), serverPort(12345), basePath("") {
    std::cout << "🐦 FlappyGame instance created" << std::endl;
}

FlappyGame::~FlappyGame() {
    // Clean up sprites
    for (auto* sprite : allSprites) {
        delete sprite;
    }
    allSprites.clear();
    std::cout << "🐦 FlappyGame instance destroyed" << std::endl;
}

void FlappyGame::InitECS() {
    std::cout << "🔧 Initializing ECS..." << std::endl;
    
    gCoordinator.Init();

    // Register components used by Lua bindings (from eng::engine::ECS namespace)
    std::cout << "[FlappyGame] Registering Transform..." << std::endl;
    gCoordinator.RegisterComponent<Transform>();               // ECS Transform (x, y, rotation) - for Lua
    std::cout << "[FlappyGame] Registering ECS::Velocity..." << std::endl;
    gCoordinator.RegisterComponent<eng::engine::ECS::Velocity>(); // ECS Velocity - for Lua
    std::cout << "[FlappyGame] Registering ECS::Sprite..." << std::endl;
    gCoordinator.RegisterComponent<eng::engine::ECS::Sprite>(); // ECS Sprite - for Lua
    std::cout << "[FlappyGame] Registering ECS::Collider..." << std::endl;
    gCoordinator.RegisterComponent<eng::engine::ECS::Collider>(); // ECS Collider - for Lua
    std::cout << "[FlappyGame] Registering Health..." << std::endl;
    gCoordinator.RegisterComponent<Health>();                  // ECS Health - for Lua
    std::cout << "[FlappyGame] Registering Damage..." << std::endl;
    gCoordinator.RegisterComponent<Damage>();                  // ECS Damage - for Lua
    std::cout << "[FlappyGame] Registering ECS::Tag..." << std::endl;
    gCoordinator.RegisterComponent<eng::engine::ECS::Tag>();   // ECS Tag (value) - for Lua
    
    // Register components used by engine systems (from components/ namespace)
    std::cout << "[FlappyGame] Registering Position..." << std::endl;
    gCoordinator.RegisterComponent<Position>();                // components Position (x, y) - for RenderSystem
    std::cout << "[FlappyGame] Registering ::Velocity..." << std::endl;
    gCoordinator.RegisterComponent<::Velocity>();              // components Velocity - for MovementSystem
    std::cout << "[FlappyGame] Registering ::Sprite..." << std::endl;
    gCoordinator.RegisterComponent<::Sprite>();                // components Sprite - for RenderSystem
    std::cout << "[FlappyGame] Registering ::Tag..." << std::endl;
    gCoordinator.RegisterComponent<::Tag>();                   // components Tag (name) - for RenderSystem
    std::cout << "[FlappyGame] Registering Animation..." << std::endl;
    gCoordinator.RegisterComponent<Animation>();               // for AnimationSystem
    std::cout << "[FlappyGame] Registering Lifetime..." << std::endl;
    gCoordinator.RegisterComponent<Lifetime>();                // for LifetimeSystem
    
    // UI components
    std::cout << "[FlappyGame] Registering UIText..." << std::endl;
    gCoordinator.RegisterComponent<Components::UIText>();
    std::cout << "[FlappyGame] Registering UIButton..." << std::endl;
    gCoordinator.RegisterComponent<Components::UIButton>();
    std::cout << "[FlappyGame] Registering UIPanel..." << std::endl;
    gCoordinator.RegisterComponent<Components::UIPanel>();

    std::cout << "[FlappyGame] Components registered (ECS + local)" << std::endl;
}

void FlappyGame::InitSystems() {
    std::cout << "🔧 Initializing Systems..." << std::endl;

    // Movement System - uses Position and ::Velocity from components/
    movementSystem = gCoordinator.RegisterSystem<MovementSystem>(&gCoordinator);
    ECS::Signature movementSig;
    movementSig.set(gCoordinator.GetComponentType<Position>());
    movementSig.set(gCoordinator.GetComponentType<::Velocity>());
    gCoordinator.SetSystemSignature<MovementSystem>(movementSig);

    // Animation System
    animationSystem = gCoordinator.RegisterSystem<AnimationSystem>();
    animationSystem->SetCoordinator(&gCoordinator);
    ECS::Signature animSig;
    animSig.set(gCoordinator.GetComponentType<Animation>());
    animSig.set(gCoordinator.GetComponentType<::Sprite>());
    gCoordinator.SetSystemSignature<AnimationSystem>(animSig);

    // Note: CollisionSystem removed - collisions are handled in Lua for Flappy Bird

    // Health System
    healthSystem = gCoordinator.RegisterSystem<HealthSystem>();
    healthSystem->SetCoordinator(&gCoordinator);
    ECS::Signature healthSig;
    healthSig.set(gCoordinator.GetComponentType<Health>());
    gCoordinator.SetSystemSignature<HealthSystem>(healthSig);

    // Lifetime System
    lifetimeSystem = gCoordinator.RegisterSystem<LifetimeSystem>(&gCoordinator);
    ECS::Signature lifetimeSig;
    lifetimeSig.set(gCoordinator.GetComponentType<Lifetime>());
    gCoordinator.SetSystemSignature<LifetimeSystem>(lifetimeSig);

    // Render System - uses Position and ::Sprite from components/
    renderSystem = gCoordinator.RegisterSystem<RenderSystem>();
    renderSystem->SetCoordinator(&gCoordinator);
    ECS::Signature renderSig;
    renderSig.set(gCoordinator.GetComponentType<Position>());
    renderSig.set(gCoordinator.GetComponentType<::Sprite>());
    gCoordinator.SetSystemSignature<RenderSystem>(renderSig);
    
    // UI System
    uiSystem = gCoordinator.RegisterSystem<UISystem>(&gCoordinator);
    ECS::Signature uiSig;
    uiSig.set(gCoordinator.GetComponentType<Position>());
    gCoordinator.SetSystemSignature<UISystem>(uiSig);

    // Initialize all systems
    movementSystem->Init();
    animationSystem->Init();
    healthSystem->Init();
    lifetimeSystem->Init();
    renderSystem->Init();
    uiSystem->Init();

    std::cout << "[FlappyGame] All Systems initialized!" << std::endl;
}

void FlappyGame::InitLua() {
    std::cout << "🌙 Initializing Lua Scripting..." << std::endl;

    auto& luaState = Scripting::LuaState::Instance();
    luaState.Init();
    luaState.EnableHotReload(true);

    // Setup Lua package.path to find our modules
    auto& lua = luaState.GetState();
    std::string scriptsPath = ResolveAssetPath("game2/assets/scripts/");
    std::string packagePath = lua["package"]["path"];
    packagePath = scriptsPath + "?.lua;" + scriptsPath + "?/init.lua;" + packagePath;
    lua["package"]["path"] = packagePath;
    std::cout << "[FlappyGame] Lua package.path updated to include: " << scriptsPath << std::endl;

    // Register all components to Lua
    Scripting::ComponentBindings::RegisterAll(luaState.GetState());
    Scripting::ComponentBindings::RegisterCoordinator(luaState.GetState(), &gCoordinator);

    // Register UI bindings
    Scripting::UIBindings::RegisterAll(luaState.GetState(), uiSystem.get());

    // Always register packet types
    NetworkBindings::RegisterPacketTypes(lua);
    
    // Register Network bindings (if network client exists)
    if (networkClient) {
        NetworkBindings::RegisterNetworkClient(lua, networkClient.get());
        std::cout << "[FlappyGame] Network bindings registered (client exists)" << std::endl;
    } else {
        // Create a stub Network table with a function to initialize the client
        lua["Network"] = lua.create_table();
        lua["Network"]["init"] = [this](const std::string& ip, int port) -> bool {
            try {
                std::cout << "[FlappyGame] Creating NetworkClient for " << ip << ":" << port << std::endl;
                networkClient = std::make_unique<NetworkClient>(ip, port);
                
                // Now register the real network bindings
                auto& luaState = Scripting::LuaState::Instance().GetState();
                NetworkBindings::RegisterNetworkClient(luaState, networkClient.get());
                std::cout << "[FlappyGame] ✅ NetworkClient created and bindings registered" << std::endl;
                return true;
            } catch (const std::exception& e) {
                std::cerr << "[FlappyGame] ❌ Failed to create NetworkClient: " << e.what() << std::endl;
                return false;
            }
        };
        std::cout << "[FlappyGame] Network stub registered (client will be created on demand)" << std::endl;
    }

    std::cout << "[FlappyGame] Lua components registered" << std::endl;

    // Create Flappy namespace in Lua (use already obtained reference)
    auto flappyTable = lua.create_table();

    // Register CreateEntity
    flappyTable.set_function("CreateEntity", [this]() -> ECS::Entity {
        auto entity = gCoordinator.CreateEntity();
        RegisterEntity(entity);
        return entity;
    });

    // Register DestroyEntity
    flappyTable.set_function("DestroyEntity", [this](ECS::Entity entity) {
        DestroyEntityDeferred(entity);
    });

    // Register SetupSprite - creates and attaches a sprite to an entity
    // Uses ::Sprite from components/ (required by RenderSystem)
    using SFMLSprite = eng::engine::rendering::sfml::SFMLSprite;
    using SFMLTexture = eng::engine::rendering::sfml::SFMLTexture;
    
    flappyTable.set_function("SetupSprite", [this](ECS::Entity entity, int textureId, int width, int height, int layer) -> bool {
        SFMLTexture* texture = nullptr;
        if (textureId == 0) texture = birdTexture.get();
        else if (textureId == 1) texture = pipeTexture.get();
        else if (textureId == 2) texture = backgroundTexture.get();
        
        if (!texture) {
            std::cerr << "[Flappy] SetupSprite: Invalid texture ID " << textureId << std::endl;
            return false;
        }

        // Create SFML sprite (raw pointer, managed by allSprites)
        auto* sfmlSprite = new SFMLSprite();
        sfmlSprite->setTexture(texture);
        sfmlSprite->setTextureRect({0, 0, width, height});
        allSprites.push_back(sfmlSprite);
        
        // Create and add ::Sprite component (for RenderSystem)
        ::Sprite spriteComp;
        spriteComp.sprite = sfmlSprite;
        spriteComp.textureRect = {0, 0, width, height};
        spriteComp.layer = layer;
        spriteComp.scaleX = 1.0f;
        spriteComp.scaleY = 1.0f;
        
        gCoordinator.AddComponent(entity, spriteComp);
        
        return true;
    });

    // Register SetPosition - adds Position component for RenderSystem
    flappyTable.set_function("SetPosition", [this](ECS::Entity entity, float x, float y) {
        if (!gCoordinator.HasComponent<Position>(entity)) {
            Position pos;
            pos.x = x;
            pos.y = y;
            gCoordinator.AddComponent(entity, pos);
        } else {
            auto& pos = gCoordinator.GetComponent<Position>(entity);
            pos.x = x;
            pos.y = y;
        }
    });

    // Register SetVelocity - adds ::Velocity component for MovementSystem
    flappyTable.set_function("SetVelocity", [this](ECS::Entity entity, float dx, float dy) {
        if (!gCoordinator.HasComponent<::Velocity>(entity)) {
            ::Velocity vel;
            vel.dx = dx;
            vel.dy = dy;
            gCoordinator.AddComponent(entity, vel);
        } else {
            auto& vel = gCoordinator.GetComponent<::Velocity>(entity);
            vel.dx = dx;
            vel.dy = dy;
        }
    });

    // Register texture IDs
    auto texturesTable = flappyTable.create_named("Textures");
    texturesTable["Bird"] = 0;
    texturesTable["Pipe"] = 1;
    texturesTable["Background"] = 2;

    // Input state table
    auto inputTable = flappyTable.create_named("Input");
    inputTable["SpaceJustPressed"] = false;
    inputTable["SpacePressed"] = false;
    inputTable["EscapeJustPressed"] = false;
    inputTable["EscapePressed"] = false;
    inputTable["MJustPressed"] = false;
    inputTable["MPressed"] = false;
    inputTable["Key1JustPressed"] = false;
    inputTable["Key1Pressed"] = false;
    inputTable["Key2JustPressed"] = false;
    inputTable["Key2Pressed"] = false;
    inputTable["Key3JustPressed"] = false;
    inputTable["Key3Pressed"] = false;

    // Screen dimensions
    flappyTable["SCREEN_WIDTH"] = 1280;
    flappyTable["SCREEN_HEIGHT"] = 720;

    // Quit function
    flappyTable.set_function("Quit", [this]() {
        shouldQuit = true;
    });

    // ===== SIMPLE UI DRAWING FUNCTIONS =====
    // Draw filled rectangle
    flappyTable.set_function("DrawRect", [this](float x, float y, float width, float height, int r, int g, int b, int a) {
        if (!currentWindow) return;
        sf::RectangleShape rect(sf::Vector2f(width, height));
        rect.setPosition(x, y);
        rect.setFillColor(sf::Color(r, g, b, a));
        currentWindow->draw(rect);
    });

    // Draw rectangle outline
    flappyTable.set_function("DrawRectOutline", [this](float x, float y, float width, float height, int r, int g, int b, int thickness) {
        if (!currentWindow) return;
        sf::RectangleShape rect(sf::Vector2f(width, height));
        rect.setPosition(x, y);
        rect.setFillColor(sf::Color::Transparent);
        rect.setOutlineColor(sf::Color(r, g, b));
        rect.setOutlineThickness(thickness);
        currentWindow->draw(rect);
    });

    // Draw text (centered at x, y)
    flappyTable.set_function("DrawText", [this](const std::string& text, float x, float y, int size, int r, int g, int b) {
        if (!currentWindow || !uiFont) return;
        sf::Text sfText;
        sfText.setFont(*uiFont);
        sfText.setString(text);
        sfText.setCharacterSize(size);
        sfText.setFillColor(sf::Color(r, g, b));
        
        // Center the text
        sf::FloatRect bounds = sfText.getLocalBounds();
        sfText.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        sfText.setPosition(x, y);
        
        currentWindow->draw(sfText);
    });

    // Draw text (left-aligned at x, y)
    flappyTable.set_function("DrawTextLeft", [this](const std::string& text, float x, float y, int size, int r, int g, int b) {
        if (!currentWindow || !uiFont) return;
        sf::Text sfText;
        sfText.setFont(*uiFont);
        sfText.setString(text);
        sfText.setCharacterSize(size);
        sfText.setFillColor(sf::Color(r, g, b));
        sfText.setPosition(x, y);
        currentWindow->draw(sfText);
    });

    // Set global
    lua["Flappy"] = flappyTable;
    lua["SCREEN_WIDTH"] = 1280;
    lua["SCREEN_HEIGHT"] = 720;

    std::cout << "[FlappyGame] Flappy bindings registered" << std::endl;

    // Load the main Lua script
    std::string luaPath = ResolveAssetPath("game2/assets/scripts/init.lua");
    std::cout << "[FlappyGame] Loading Lua script: " << luaPath << std::endl;
    
    try {
        luaState.GetState().script_file(luaPath);
        std::cout << "[FlappyGame] ✅ Lua script loaded successfully!" << std::endl;
    } catch (const sol::error& e) {
        std::cerr << "[FlappyGame] ❌ Lua error: " << e.what() << std::endl;
    }
}

void FlappyGame::LoadAssets() {
    std::cout << "🎨 Loading Assets..." << std::endl;

    birdTexture = std::make_unique<SFMLTexture>();
    if (!birdTexture->loadFromFile(ResolveAssetPath("game2/assets/sprites/bird.png"))) {
        std::cerr << "Warning: Could not load bird.png" << std::endl;
    } else {
        std::cout << "[FlappyGame] ✅ Bird texture loaded" << std::endl;
    }

    pipeTexture = std::make_unique<SFMLTexture>();
    if (!pipeTexture->loadFromFile(ResolveAssetPath("game2/assets/sprites/pipe.png"))) {
        std::cerr << "Warning: Could not load pipe.png" << std::endl;
    } else {
        std::cout << "[FlappyGame] ✅ Pipe texture loaded" << std::endl;
    }

    backgroundTexture = std::make_unique<SFMLTexture>();
    if (!backgroundTexture->loadFromFile(ResolveAssetPath("game2/assets/sprites/background.png"))) {
        std::cerr << "Warning: Could not load background.png" << std::endl;
    } else {
        std::cout << "[FlappyGame] ✅ Background texture loaded" << std::endl;
    }

    // Load UI font for simple drawing
    uiFont = std::make_unique<sf::Font>();
    std::string fontPath = ResolveAssetPath("game2/assets/fonts/Arial.ttf");
    if (!uiFont->loadFromFile(fontPath)) {
        std::cerr << "[FlappyGame] ⚠️ Warning: Could not load UI font: " << fontPath << std::endl;
    } else {
        std::cout << "[FlappyGame] ✅ UI font loaded for simple drawing" << std::endl;
    }
}

void FlappyGame::ProcessEvents(SFMLWindow& window) {
    // Reset "just pressed" states
    auto& luaState = Scripting::LuaState::Instance();
    auto& lua = luaState.GetState();
    lua["Flappy"]["Input"]["SpaceJustPressed"] = false;
    lua["Flappy"]["Input"]["EscapeJustPressed"] = false;
    lua["Flappy"]["Input"]["MJustPressed"] = false;
    lua["Flappy"]["Input"]["Key1JustPressed"] = false;
    lua["Flappy"]["Input"]["Key2JustPressed"] = false;
    lua["Flappy"]["Input"]["Key3JustPressed"] = false;

    bool spaceIsPressed = false;
    bool escapeIsPressed = false;
    bool mIsPressed = false;
    bool key1IsPressed = false;
    bool key2IsPressed = false;
    bool key3IsPressed = false;

    eng::engine::InputEvent event;
    while (window.pollEvent(event)) {
        if (event.type == eng::engine::EventType::Closed) {
            window.close();
        }

        // Track key states
        if (event.type == eng::engine::EventType::KeyPressed) {
            if (event.key.code == eng::engine::Key::Space) {
                spaceIsPressed = true;
                if (!spaceWasPressed) {
                    lua["Flappy"]["Input"]["SpaceJustPressed"] = true;
                }
            }
            if (event.key.code == eng::engine::Key::Escape) {
                escapeIsPressed = true;
                if (!escapeWasPressed) {
                    lua["Flappy"]["Input"]["EscapeJustPressed"] = true;
                }
            }
            if (event.key.code == eng::engine::Key::M) {
                mIsPressed = true;
                if (!mWasPressed) {
                    lua["Flappy"]["Input"]["MJustPressed"] = true;
                }
            }
            if (event.key.code == eng::engine::Key::Num1) {
                key1IsPressed = true;
                if (!key1WasPressed) {
                    lua["Flappy"]["Input"]["Key1JustPressed"] = true;
                }
            }
            if (event.key.code == eng::engine::Key::Num2) {
                key2IsPressed = true;
                if (!key2WasPressed) {
                    lua["Flappy"]["Input"]["Key2JustPressed"] = true;
                }
            }
            if (event.key.code == eng::engine::Key::Num3) {
                key3IsPressed = true;
                if (!key3WasPressed) {
                    lua["Flappy"]["Input"]["Key3JustPressed"] = true;
                }
            }

            // Forward to Lua
            sol::function onKeyPressed = lua["OnKeyPressed"];
            if (onKeyPressed.valid()) {
                try {
                    onKeyPressed(static_cast<int>(event.key.code));
                } catch (const sol::error& e) {
                    std::cerr << "[Lua] OnKeyPressed error: " << e.what() << std::endl;
                }
            }
        }

        if (event.type == eng::engine::EventType::KeyReleased) {
            if (event.key.code == eng::engine::Key::Space) {
                spaceIsPressed = false;
            }
            if (event.key.code == eng::engine::Key::Escape) {
                escapeIsPressed = false;
            }
            if (event.key.code == eng::engine::Key::M) {
                mIsPressed = false;
            }
            if (event.key.code == eng::engine::Key::Num1) {
                key1IsPressed = false;
            }
            if (event.key.code == eng::engine::Key::Num2) {
                key2IsPressed = false;
            }
            if (event.key.code == eng::engine::Key::Num3) {
                key3IsPressed = false;
            }

            // Forward to Lua
            sol::function onKeyReleased = lua["OnKeyReleased"];
            if (onKeyReleased.valid()) {
                try {
                    onKeyReleased(static_cast<int>(event.key.code));
                } catch (const sol::error& e) {
                    std::cerr << "[Lua] OnKeyReleased error: " << e.what() << std::endl;
                }
            }
        }
    }

    // Update input state in Lua
    lua["Flappy"]["Input"]["SpacePressed"] = spaceIsPressed;
    lua["Flappy"]["Input"]["EscapePressed"] = escapeIsPressed;
    lua["Flappy"]["Input"]["MPressed"] = mIsPressed;
    lua["Flappy"]["Input"]["Key1Pressed"] = key1IsPressed;
    lua["Flappy"]["Input"]["Key2Pressed"] = key2IsPressed;
    lua["Flappy"]["Input"]["Key3Pressed"] = key3IsPressed;
    
    spaceWasPressed = spaceIsPressed;
    escapeWasPressed = escapeIsPressed;
    mWasPressed = mIsPressed;
    key1WasPressed = key1IsPressed;
    key2WasPressed = key2IsPressed;
    key3WasPressed = key3IsPressed;
}

void FlappyGame::Update(float dt) {
    // Update ECS systems
    movementSystem->Update(dt);
    animationSystem->Update(dt);
    healthSystem->Update(dt);
    lifetimeSystem->Update(dt);
    uiSystem->Update(dt);

    // Call Lua Update function
    auto& luaState = Scripting::LuaState::Instance();
    sol::function luaUpdate = luaState.GetState()["Update"];
    if (luaUpdate.valid()) {
        try {
            luaUpdate(dt);
        } catch (const sol::error& e) {
            std::cerr << "[Lua] Update error: " << e.what() << std::endl;
        }
    }

    // Process destroyed entities
    ProcessDestroyedEntities();
}

void FlappyGame::Render(SFMLRenderer& /*renderer*/) {
    renderSystem->Update(0.0f);

    // Call Lua Render function (for UI, etc.)
    auto& luaState = Scripting::LuaState::Instance();
    sol::function luaRender = luaState.GetState()["Render"];
    if (luaRender.valid()) {
        try {
            luaRender();
        } catch (const sol::error& e) {
            std::cerr << "[Lua] Render error: " << e.what() << std::endl;
        }
    }
}

int FlappyGame::Run(int argc, char* argv[]) {
    std::cout << "🐦 Flappy Bird Battle Royale Starting..." << std::endl;

    // Parse command line arguments for network mode
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--network" || arg == "-n") && i + 2 < argc) {
            serverAddress = argv[i + 1];
            serverPort = static_cast<short>(std::atoi(argv[i + 2]));
            std::cout << "[FlappyGame] Network mode enabled: " << serverAddress << ":" << serverPort << std::endl;
            
            // Create network client
            try {
                networkClient = std::make_unique<NetworkClient>(serverAddress, serverPort);
                std::cout << "[FlappyGame] ✅ NetworkClient created" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "[FlappyGame] ❌ Failed to create NetworkClient: " << e.what() << std::endl;
                networkClient = nullptr;
            }
            
            i += 2;  // Skip the next two arguments
        }
    }

    // Initialize ECS
    InitECS();

    // Initialize Systems
    InitSystems();

    // Initialize Lua
    InitLua();

    // Create window
    SFMLWindow window;
    window.create(1280, 720, "Flappy Bird Battle Royale");

    SFMLRenderer renderer(&window.getSFMLWindow());

    // Set renderer for RenderSystem
    renderSystem->SetRenderer(&renderer);

    // Load assets
    LoadAssets();

    // Call Lua Init function
    auto& luaState = Scripting::LuaState::Instance();
    sol::function luaInit = luaState.GetState()["Init"];
    if (luaInit.valid()) {
        try {
            luaInit();
            std::cout << "[FlappyGame] ✅ Lua Init() called" << std::endl;
        } catch (const sol::error& e) {
            std::cerr << "[Lua] Init error: " << e.what() << std::endl;
        }
    }

    // Game clock
    eng::engine::Clock clock;

    std::cout << "[FlappyGame] 🎮 Entering main game loop..." << std::endl;

    // Main game loop
    while (window.isOpen() && !shouldQuit) {
        float dt = clock.restart();

        // Process events
        ProcessEvents(window);

        // Update
        Update(dt);

        // Render
        window.clear();
        
        // Set current window for UI drawing
        currentWindow = &window.getSFMLWindow();  // Get underlying sf::RenderWindow
        
        Render(renderer);
        
        // Call Lua RenderUI for simple overlay UI
        auto& lua = luaState.GetState();
        sol::function renderUI = lua["RenderUI"];
        if (renderUI.valid()) {
            try {
                renderUI();
            } catch (const sol::error& e) {
                std::cerr << "[Lua] RenderUI error: " << e.what() << std::endl;
            }
        }
        
        currentWindow = nullptr;  // Clear for safety
        
        window.display();
    }

    std::cout << "[FlappyGame] 👋 Game ended" << std::endl;
    return 0;
}

} // namespace FlappyBird
