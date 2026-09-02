#pragma once

#include "ui/saveable.hpp"

struct EditorCommands;
struct GameData;
class Camera2D;

class CameraUi
{
public:
    void draw(GameData &gameData, const Camera2D &camera, EditorCommands &commands);
    void save(GameData &gameData);
    void revert(GameData &gameData);
    bool hasUnsavedChanges(const GameData &gameData);
    void valuesReplaced();

private:
    Saveable saveable;
};
