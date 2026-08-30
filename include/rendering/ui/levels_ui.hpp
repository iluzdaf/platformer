#pragma once

#include "rendering/ui/saveable.hpp"

class Level;
class Levels;
struct EditorCommands;

class LevelsUi
{
public:
    void draw(Levels &levels, const Level &level, EditorCommands &commands);
    void valuesReplaced();

private:
    Saveable saveable;
};
