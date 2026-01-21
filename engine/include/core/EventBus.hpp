/*
** EPITECH PROJECT, 2025
** Eventbus
** File description:
** engine
*/

#ifndef EVENTBUS_CORE
#define EVENTBUS_CORE

#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace eng {
namespace core {

class EventBus {
public:
    EventBus() = default;
    ~EventBus() = default;

    // Subscribe to an event type
    template <typename T>
    void subscribe(std::function<void(const T&)> callback);

    // Publish an event to all subscribers
    template <typename T>
    void publish(const T& event);

    // Clear all subscribers
    void clear();

protected:
private:
    std::unordered_map<std::type_index, std::vector<std::function<void(const void*)>>> _subscribers;
};

}  // namespace core
}  // namespace eng

#endif /* !EVENTBUS_CORE */
