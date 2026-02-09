/*
** EPITECH PROJECT, 2025
** Event
** File description:
** engine
*/

#ifndef _EVENT_CORE_
    #define _EVENT_CORE_

    #include <cstdint>
    #include "Types.hpp"

    namespace eng {
        namespace core {

            // Base event class
            struct Event {
                virtual ~Event() = default;
            };

            // Collision event
            struct CollisionEvent : public Event {
                uint32_t entityA;
                uint32_t entityB;
            };

            // Input event
            struct InputEvent : public Event {
                uint32_t entityId;
                int inputType; // e.g., key press, mouse click
            };

            // Entity destroyed event
            struct EntityDestroyedEvent : public Event {
                uint32_t entityId;
            };

            // Client connected event
            struct ClientConnectedEvent : public Event {
                uint32_t clientId;
            };

            // Client disconnected event
            struct ClientDisconnectedEvent : public Event {
                uint32_t clientId;
            };

            // Window event
            struct WindowEvent : public Event {
                enum Type { Closed, Resized, LostFocus, GainedFocus };
                Type type;
                uint32_t width;   // pour Resized
                uint32_t height;  // pour Resized
                
                WindowEvent(Type t = Closed, uint32_t w = 0, uint32_t h = 0) 
                    : type(t), width(w), height(h) {}
            };

            // Network event
            struct NetworkEvent : public Event {
                enum Type { PacketReceived, ConnectionLost, ServerTimeout };
                Type type;
                uint32_t connectionId;
                
                NetworkEvent(Type t, uint32_t id = 0) 
                    : type(t), connectionId(id) {}
            };

            // Entity spawned event
            struct EntitySpawnedEvent : public Event {
                uint32_t entityId;
                eng::engine::Vector2i spawnPosition;
            };

            // Health changed event
            struct HealthChangedEvent : public Event {
                uint32_t entityId;
                int oldHealth;
                int newHealth;
            };

        } // namespace core
    } // namespace eng

#endif /* !_EVENT_CORE_ */
