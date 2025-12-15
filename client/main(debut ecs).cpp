#include <ecs/Coordinator.hpp>
#include <ecs/RegisterCoreComponents.hpp>
#include <systems/RenderSystem.hpp>
#include <rendering/sfml/SFMLWindow.hpp>
#include <rendering/sfml/SFMLRenderer.hpp>
#include <rendering/sfml/SFMLTexture.hpp>
#include <rendering/sfml/SFMLSprite.hpp>
#include <components/Position.hpp>
#include <components/Sprite.hpp>
#include <SFML/System/Clock.hpp>
#include <iostream>

int main()
{
    // 1) ECS init + register core components
    ECS::Coordinator coordinator;
    coordinator.Init();
    coordinator.RegisterDefaultComponents(); // Position, Velocity, Sprite

    // 2) Window & renderer (engine SFML wrappers)
    rtype::engine::rendering::sfml::SFMLWindow window;
    window.create(800, 600, "R-Type - Demo");
    rtype::engine::rendering::sfml::SFMLRenderer renderer(&window.getSFMLWindow());

    // 3) Register RenderSystem
    auto renderSys = coordinator.RegisterSystem<RenderSystem>();
    renderSys->SetCoordinator(&coordinator);
    renderSys->SetRenderer(&renderer);

    // 4) Set system signature (Position + Sprite)
    ECS::Signature renderSig;
    renderSig.set(coordinator.GetComponentType<Position>(), true);
    renderSig.set(coordinator.GetComponentType<Sprite>(), true);
    coordinator.SetSystemSignature<RenderSystem>(renderSig);

    // 5) Create an entity + components
    ECS::Entity player = coordinator.CreateEntity();
    coordinator.AddComponent<Position>(player, Position{400.0f, 300.0f});

    // // Load texture and create sprite (quick & dirty)
    // using namespace rtype::engine::rendering::sfml;
    // SFMLTexture *tex = new SFMLTexture();
    // if (!tex->loadFromFile("assets/textures/player.png")) {
    //     std::cerr << "Failed to load texture\n";
    //     return 1;
    // }

    // SFMLSprite *nativeSprite = new SFMLSprite();
    // nativeSprite->setTexture(tex);
    // nativeSprite->setPosition({400.0f, 300.0f});

    // Sprite spriteComp;
    // spriteComp.sprite = nativeSprite;
    // coordinator.AddComponent<Sprite>(player, spriteComp);

    // 6) Main loop
    sf::Clock clock;
    while (window.isOpen()) {
        sf::Event ev;

        while (window.pollEvent(ev)) {
            if (ev.type == sf::Event::Closed)
                window.close();
        }

        float dt = clock.restart().asSeconds();

        renderer.clear();
        // update systems manually (we have the shared_ptr)
        renderSys->Update(dt);
        renderer.display();
    }

    // cleanup (free allocations created with new)
    // delete nativeSprite;
    // delete tex;

    return 0;
}