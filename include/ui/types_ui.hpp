#pragma once

#include "ui/saveable.hpp"
#include "ui/type_shown.hpp"

struct GameData;

class TypesUi
{
public:
    void draw(GameData &gameData);
    void save(GameData &gameData);
    void revert(GameData &gameData);
    bool unsavedSince(const GameData &gameData);
    void valuesReplaced();

private:
    void drawChooser(GameData &gameData);

    Saveable saveable;
    TypeShown showing;
};
