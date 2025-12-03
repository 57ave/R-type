/*
** EPITECH PROJECT, 2025
** Event
** File description:
** rtype
*/

#ifndef _EVENT_CORE_
    #define _EVENT_CORE_

struct CollisionEvent {
    int entityA;
    int entityB;
};

struct InputEvent
{
    int entityId;
    int inputType; // e.g., key press, mouse click
};

struct EntityDestroyedEvent {
    int entityId;
};

struct ClientConnectedEvent {
    int clientId;
};

struct ClientDisconnectedEvent {
    int clientId;
};

#endif /* !_EVENT_CORE_ */
