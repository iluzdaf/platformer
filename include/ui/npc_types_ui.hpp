#pragma once

#include "ui/saveable.hpp"

struct GameData;

class NpcTypesUi
{
public:
    void draw(GameData &gameData);
    bool hasUnsavedChanges(const GameData &gameData) const;
    void valuesReplaced();

private:
    Saveable saveable;
};
