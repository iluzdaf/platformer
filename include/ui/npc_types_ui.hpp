#pragma once

#include "ui/saveable.hpp"

struct GameData;

class NpcTypesUi
{
public:
    void draw(GameData &gameData);
    void valuesReplaced();

private:
    Saveable saveable;
};
