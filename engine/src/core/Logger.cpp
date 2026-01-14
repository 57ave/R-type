/*
** EPITECH PROJECT, 2025
** engine
** File description:
** looger core
*/
#include "core/Logger.hpp"

void eng::core::Logger::info(std::string const &message) {
    std::cout << "[INFO]: " << message << std::endl;
}

void eng::core::Logger::warning(std::string const &message) {
    std::cout << "[WARNING]: " << message << std::endl;
}
void eng::core::Logger::error(std::string const &message) {
    std::cout << "[ERROR]: " << message << std::endl;
}
void eng::core::Logger::debug(std::string const &message){
    std::cout << "[DEBUG]: " << message << std::endl;
}



