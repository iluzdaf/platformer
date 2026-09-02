#pragma once

#include "ui/saveable.hpp"

struct EditorCommands;
struct GameData;

class GameSettingsUi
{
public:
    void draw(GameData &gameData, EditorCommands &commands);
    void save(GameData &gameData);
    void revert(GameData &gameData);
    bool unsavedSince(const GameData &gameData);
    void valuesReplaced();

private:
    Saveable saveable;
};
