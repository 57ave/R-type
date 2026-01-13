#ifndef RTYPE_ENGINE_SCRIPTING_UIBINDINGS_HPP
#define RTYPE_ENGINE_SCRIPTING_UIBINDINGS_HPP

#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

// Forward declarations
class UISystem;
class GameStateManager;

namespace Scripting {

    /**
     * @brief Lua bindings for the UI system
     * 
     * Exposes UI creation and manipulation functions to Lua scripts.
     * 
     * Usage in Lua:
     *   -- Create UI elements
     *   local btn = UI.CreateButton({ x=400, y=300, width=200, height=50, text="Play", onClick="OnPlay" })
     *   local txt = UI.CreateText({ x=400, y=100, text="R-TYPE", fontSize=72 })
     *   
     *   -- Manipulate UI
     *   UI.SetVisible(btn, false)
     *   UI.SetText(txt, "New Text")
     *   
     *   -- Menu management
     *   UI.ShowMenu("main")
     *   UI.HideMenu("pause")
     *   
     *   -- Navigation
     *   UI.SelectNext()
     *   UI.ActivateSelected()
     */
    class UIBindings {
    public:
        /**
         * @brief Register all UI bindings to Lua
         * @param lua The Lua state
         * @param uiSystem Pointer to the UISystem (can be set later)
         */
        static void RegisterAll(sol::state& lua, UISystem* uiSystem = nullptr);

        /**
         * @brief Set the UISystem reference (call after UISystem is created)
         */
        static void SetUISystem(UISystem* uiSystem);

        /**
         * @brief Register GameState bindings
         */
        static void RegisterGameState(sol::state& lua);

    private:
        static UISystem* s_uiSystem;
    };

} // namespace Scripting

#endif // RTYPE_ENGINE_SCRIPTING_UIBINDINGS_HPP
