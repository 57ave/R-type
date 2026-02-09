/**
 * R-Type Game - Main Entry Point
 * 
 * This is the main entry point for the R-Type game.
 * It creates and runs the Game instance.
 */

#include "core/Game.hpp"
#include <iostream>
#include <exception>

int main()
{
    try
    {
        Game game;
        
        if (!game.initialize())
        {
            std::cerr << "[MAIN] Failed to initialize game" << std::endl;
            return EXIT_FAILURE;
        }
        
        game.run();
        game.shutdown();
        
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[MAIN] Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
