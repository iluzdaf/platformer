#pragma once

#include "ui/saveable.hpp"

struct EditorCommands;
struct GameData;

class GameSettingsUi
{
public:
    void draw(GameData &gameData, EditorCommands &commands);
    void save(GameData &gameData);
    bool hasUnsavedChanges(const GameData &gameData) const;
    void valuesReplaced();

private:
    Saveable saveable;
};
