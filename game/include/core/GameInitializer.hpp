#pragma once

#include <ecs/Coordinator.hpp>
#include <rendering/sfml/SFMLRenderer.hpp>
#include <scripting/LuaState.hpp>
#include <sol/sol.hpp>
#include <memory>

// Forward declarations
class UISystem;

namespace RType::Core {

/**
 * @brief Responsable de l'initialisation de tous les composants du moteur ECS
 * 
 * Cette classe gère :
 * - Initialisation du coordinateur ECS
 * - Enregistrement des composants
 * - Enregistrement des systèmes 
 * - Configuration Lua
 * - Bindings de scripting
 */
class GameInitializer {
public:
    /**
     * @brief Initialise le coordinateur ECS et tous ses composants
     * @param coordinator Référence vers le coordinateur à initialiser
     * @param renderer Référence vers le renderer SFML
     * @param luaState Référence vers l'état Lua
     * @return true si l'initialisation réussit, false sinon
     */
    static bool InitializeECS(ECS::Coordinator& coordinator, 
                             eng::engine::rendering::sfml::SFMLRenderer& renderer,
                             ::Scripting::LuaState& luaState);

    /**
     * @brief Enregistre tous les composants dans l'ECS
     * @param coordinator Référence vers le coordinateur ECS
     */
    static void RegisterComponents(ECS::Coordinator& coordinator);

    /**
     * @brief Enregistre tous les systèmes dans l'ECS
     * @param coordinator Référence vers le coordinateur ECS
     * @param renderer Référence vers le renderer
     * @param outUISystem Pointeur optionnel pour récupérer le UISystem
     * @return true si tous les systèmes sont enregistrés avec succès
     */
    static bool RegisterSystems(ECS::Coordinator& coordinator,
                               eng::engine::rendering::sfml::SFMLRenderer& renderer,
                               std::shared_ptr<UISystem>* outUISystem = nullptr);

    /**
     * @brief Configure les bindings Lua et charge les scripts
     * @param luaState Référence vers l'état Lua
     * @param coordinator Référence vers le coordinateur ECS
     * @return true si la configuration Lua réussit
     */
    static bool SetupLuaBindings(::Scripting::LuaState& luaState, 
                                ECS::Coordinator& coordinator);

private:
    GameInitializer() = delete;  // Classe utilitaire uniquement
};

} // namespace RType::Core
