#pragma once

#include "game/game.hpp"
#include "window/window.hpp"

class App
{
public:
    App();
    void run();

private:
    Window window;
    Game game;
};
