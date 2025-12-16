/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** looger core
*/
#include "core/Logger.hpp"

void rtype::core::Logger::info(std::string const &message) {
    std::cout << "[INFO]: " << message << std::endl;
}

void rtype::core::Logger::warning(std::string const &message) {
    std::cout << "[WARNING]: " << message << std::endl;
}
void rtype::core::Logger::error(std::string const &message) {
    std::cout << "[ERROR]: " << message << std::endl;
}
void rtype::core::Logger::debug(std::string const &message){
    std::cout << "[DEBUG]: " << message << std::endl;
}



