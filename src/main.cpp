#include <iostream>
#include "game/game.hpp"

int main()
{
    try
    {
        Game game;
        game.run();
    }
    catch (const sol::error &e)
    {
        std::cerr << "Lua error: " << e.what() << std::endl;
        return -1;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught" << std::endl;
        return -1;
    }

    return 0;
}