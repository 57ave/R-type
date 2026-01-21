#include "GameRefactored.hpp"
#include <iostream>
#include <exception>

int main(int argc, char* argv[])
{
    try {
        RType::GameRefactored game;
        return game.Run(argc, argv);
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return 2;
    }
}
