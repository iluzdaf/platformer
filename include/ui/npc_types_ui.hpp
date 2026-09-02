#pragma once

#include "ui/saveable.hpp"

struct GameData;

class NpcTypesUi
{
public:
    void draw(GameData &gameData);
    void save(GameData &gameData);
    void revert(GameData &gameData);
    bool unsavedSince(const GameData &gameData);
    void valuesReplaced();

private:
    Saveable saveable;
};
