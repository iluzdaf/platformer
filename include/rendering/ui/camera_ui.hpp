#pragma once

#include "rendering/ui/saveable.hpp"

struct EditorCommands;
struct GameData;
class Camera2D;

class CameraUi
{
public:
    void draw(GameData &gameData, const Camera2D &camera, EditorCommands &commands);
    void valuesReplaced();

private:
    Saveable saveable;
};
