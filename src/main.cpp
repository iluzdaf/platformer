#include <iostream>
#include "game/game.hpp"

int main()
{
    try
    {
        Game game;
        game.run();
    }
    catch (const sol::error& e)
    {
        std::cerr << "Lua error: " << e.what() << std::endl;
    }
    catch (const std::runtime_error &e)
    {
        std::cout << e.what() << "\n";
        return -1;
    }

    return 0;
}