#include "FlappyGame.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    std::cout << "========================================" << std::endl;
    std::cout << "  🐦 FLAPPY BIRD - BATTLE ROYALE 🐦" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage: flappy_bird [--network <ip> <port>]" << std::endl;
    std::cout << std::endl;

    FlappyBird::FlappyGame game;
    return game.Run(argc, argv);
}
