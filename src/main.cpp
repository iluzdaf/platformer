#include <iostream>
#include <sol/sol.hpp>
#include "app/app.hpp"

int main()
{
    try
    {
        App app;
        app.run();
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
