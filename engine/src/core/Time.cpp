/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** time
*/

#include "core/Time.hpp"

long long rtype::core::Time::getElapsedTime() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - _startTime).count();
}

void rtype::core::Time::reset() {
    _startTime = std::chrono::high_resolution_clock::now();
}

void rtype::core::Time::update() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float deltaTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTime - _startTime).count();
    _deltaTime = deltaTime * _timeScale;
    _totalTime += _deltaTime;
    _startTime = currentTime;
}