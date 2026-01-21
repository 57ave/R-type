#include "GameRefactored.hpp"
#include <iostream>
#include <exception>

int main(void)
{
    try {
        RType::GameRefactored game;
        return game.Run();
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
