#include <exception>
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
        std::cerr << "Lua error: " << e.what() << '\n';
        return -1;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return -1;
    }
    catch (...)
    {
        std::cerr << "Unknown exception caught" << '\n';
        return -1;
    }

    return 0;
}
