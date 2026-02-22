/**
 * R-Type Game - Main Entry Point
 * 
 * This is the main entry point for the R-Type game.
 * It creates and runs the Game instance.
 */

#include "core/Game.hpp"
#include "core/Logger.hpp"
#include <exception>

int main()
{
    try
    {
        Game game;
        
        if (!game.initialize())
        {
            LOG_ERROR("MAIN", "[MAIN] Failed to initialize game");
            return EXIT_FAILURE;
        }
        
        game.run();
        game.shutdown();
        
        return EXIT_SUCCESS;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR("MAIN", std::string("[MAIN] Fatal error: ") + e.what());
        return EXIT_FAILURE;
    }
}
