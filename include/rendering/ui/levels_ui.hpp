#pragma once

#include "rendering/ui/saveable.hpp"

class Levels;

class LevelsUi
{
public:
    void draw(Levels &levels);
    void valuesReplaced();

private:
    Saveable saveable;
};
