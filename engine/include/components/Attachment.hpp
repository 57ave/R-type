#ifndef RTYPE_ENGINE_COMPONENTS_ATTACHMENT_HPP
#define RTYPE_ENGINE_COMPONENTS_ATTACHMENT_HPP

#include <ecs/ECS.hpp>

/**
 * @brief Composant pour attacher une entité à une autre (parent-enfant)
 * 
 * Utilisé pour les weapon attachments visuels, les options du vaisseau,
 * les effets qui suivent le joueur, etc.
 */
struct Attachment {
    ECS::Entity parent = 0;        // Entité parente (ex: le vaisseau)
    
    // Position relative par rapport au parent
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    
    // Rotation relative (en degrés)
    float rotation = 0.0f;
    
    // Point d'attachement sur le parent
    enum class AttachmentPoint {
        CENTER,          // Centre du vaisseau
        LEFT_WING,       // Aile gauche
        RIGHT_WING,      // Aile droite
        CENTER_TOP,      // Dessus du vaisseau
        CENTER_BOTTOM,   // Dessous du vaisseau
        FRONT,           // Avant du vaisseau
        BACK,            // Arrière du vaisseau
        CUSTOM           // Position custom définie par offsetX/offsetY
    };
    AttachmentPoint point = AttachmentPoint::CUSTOM;
    
    // Comportement
    bool inheritRotation = true;   // Suit la rotation du parent
    bool inheritScale = true;      // Suit le scale du parent
    bool destroyWithParent = true; // Détruit quand le parent est détruit
};

/**
 * @brief Composant spécifique pour les weapon attachments visuels
 * 
 * Représente l'apparence visuelle des armes attachées au vaisseau
 */
struct WeaponAttachment {
    enum class VisualType {
        CANNON,          // Canon standard
        DOUBLE_CANNON,   // Double canon
        LASER_BARREL,    // Canon laser
        MISSILE_POD,     // Pod à missiles
        ENERGY_ORB,      // Orbe d'énergie
        PLASMA_EMITTER,  // Émetteur de plasma
        BEAM_GENERATOR   // Générateur de faisceau
    };
    
    VisualType visualType = VisualType::CANNON;
    int level = 1;                 // Niveau de l'arme (change l'apparence)
    
    // Animation
    bool animated = false;         // L'attachment a une animation
    float animationSpeed = 1.0f;   // Vitesse d'animation
    
    // Effets visuels
    bool glowing = false;          // Effet de glow
    float glowIntensity = 1.0f;    // Intensité du glow
};

#endif // RTYPE_ENGINE_COMPONENTS_ATTACHMENT_HPP
