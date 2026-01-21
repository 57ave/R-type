#pragma once

#include <memory>
#include <unordered_map>
#include <functional>
#include <cstdint>
#include <engine/Input.hpp>

namespace RType::Core {

// Enumération des actions possibles dans le jeu
enum class InputAction {
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    Shoot,
    Pause,
    Confirm,     // Pour les menus (Enter/Space)
    Cancel,      // Pour les menus (Escape)
    Console,     // Ouvrir la console de dev
    Menu         // Retour au menu
};

// Structure pour stocker l'état d'une action
struct ActionState {
    bool pressed = false;
    bool justPressed = false;
    bool justReleased = false;
    float holdTime = 0.0f;
};

/**
 * @brief Gestionnaire centralisé des entrées utilisateur
 * 
 * Cette classe gère toutes les entrées clavier/souris/manette et les convertit
 * en actions de jeu abstraites. Elle supporte :
 * - Mapping configurable des touches
 * - Détection des pressions/relâchements
 * - Temps de maintien des touches
 * - Callbacks pour les événements
 * - Masque d'entrée pour le réseau
 */
class InputHandler {
public:
    InputHandler();
    ~InputHandler();

    // ========================================
    // CONFIGURATION
    // ========================================
    
    /**
     * @brief Configure le mapping des touches pour une action
     * @param action L'action à mapper
     * @param keyCode Le code de la touche
     */
    void MapKey(InputAction action, eng::engine::Key keyCode);
    
    /**
     * @brief Charge la configuration des touches depuis un fichier Lua
     * @param configPath Chemin vers le fichier de configuration
     */
    bool LoadKeyMapping(const std::string& configPath);
    
    /**
     * @brief Sauvegarde la configuration actuelle
     * @param configPath Chemin de sauvegarde
     */
    void SaveKeyMapping(const std::string& configPath);

    // ========================================
    // MISE À JOUR
    // ========================================
    
    /**
     * @brief Met à jour l'état des entrées
     * @param deltaTime Temps écoulé depuis la dernière frame
     */
    void Update(float deltaTime);
    
    /**
     * @brief Traite un événement d'entrée
     * @param event L'événement à traiter
     */
    void HandleEvent(const eng::engine::InputEvent& event);

    // ========================================
    // ÉTAT DES ACTIONS
    // ========================================
    
    /**
     * @brief Vérifie si une action est actuellement pressée
     */
    bool IsActionPressed(InputAction action) const;
    
    /**
     * @brief Vérifie si une action vient d'être pressée (ce frame)
     */
    bool IsActionJustPressed(InputAction action) const;
    
    /**
     * @brief Vérifie si une action vient d'être relâchée (ce frame)
     */
    bool IsActionJustReleased(InputAction action) const;
    
    /**
     * @brief Obtient le temps de maintien d'une action
     */
    float GetActionHoldTime(InputAction action) const;

    // ========================================
    // CALLBACKS
    // ========================================
    
    /**
     * @brief Définit un callback pour une action
     */
    void SetActionCallback(InputAction action, std::function<void()> callback);
    
    /**
     * @brief Supprime le callback d'une action
     */
    void RemoveActionCallback(InputAction action);

    // ========================================
    // RÉSEAU
    // ========================================
    
    /**
     * @brief Génère un masque d'entrée pour la synchronisation réseau
     * @return Masque de bits représentant l'état des entrées
     */
    uint8_t GetNetworkInputMask() const;
    
    /**
     * @brief Applique un masque d'entrée reçu du réseau
     * @param mask Le masque à appliquer
     */
    void ApplyNetworkInputMask(uint8_t mask);

    // ========================================
    // CONTRÔLEUR DE MOUVEMENT
    // ========================================
    
    /**
     * @brief Obtient le vecteur de mouvement normalisé
     * @return Vecteur 2D de mouvement (-1 à 1 pour chaque axe)
     */
    std::pair<float, float> GetMovementVector() const;

    // ========================================
    // DEBUG
    // ========================================
    
    /**
     * @brief Active/désactive les logs de debug
     */
    void SetDebugMode(bool enabled);
    
    /**
     * @brief Affiche l'état actuel de toutes les actions
     */
    void DebugPrintState() const;

private:
    // État des actions
    std::unordered_map<InputAction, ActionState> actionStates;
    
    // Mapping touches -> actions
    std::unordered_map<eng::engine::Key, InputAction> keyToAction;
    
    // Callbacks par action
    std::unordered_map<InputAction, std::function<void()>> actionCallbacks;
    
    // Flags internes
    bool debugMode;
    
    // Méthodes privées
    void UpdateActionState(InputAction action, bool pressed, float deltaTime);
    void TriggerActionCallback(InputAction action);
    std::string ActionToString(InputAction action) const;
    std::string KeyToString(eng::engine::Key key) const;
};

} // namespace RType::Core
