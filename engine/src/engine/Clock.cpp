/*
** EPITECH PROJECT, 2025
** Clock
** File description:
** Clock implementation with SFML backend
*/

#include "engine/Clock.hpp"
#include <SFML/System/Clock.hpp>

namespace rtype {
    namespace engine {
        namespace internal {

            class ClockImpl {
            public:
                sf::Clock clock;
            };

        } // namespace internal

        Clock::Clock() 
            : m_impl(std::make_unique<internal::ClockImpl>()) {
        }

        Clock::~Clock() = default;

        float Clock::restart() {
            return m_impl->clock.restart().asSeconds();
        }

        float Clock::getElapsedTime() const {
            return m_impl->clock.getElapsedTime().asSeconds();
        }

    } // namespace engine
} // namespace rtype
