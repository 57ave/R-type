/*
** EPITECH PROJECT, 2025
** engine
** File description:
** EventBus
*/

#include "core/EventBus.hpp"

template<typename T>
void eng::core::EventBus::subscribe(std::function<void(const T&)> callback) {
    auto& subscribers = _subscribers[typeid(T)];
    subscribers.push_back([callback](const void* event) {
        callback(*static_cast<const T*>(event));
    });
}

template<typename T>
void eng::core::EventBus::publish(const T& event) {
    auto it = _subscribers.find(typeid(T));
    if (it != _subscribers.end()) {
        for (const auto& callback : it->second) {
            callback(&event);
        }
    }
}

void eng::core::EventBus::clear() {
    _subscribers.clear();
}