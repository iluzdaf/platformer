#pragma once

#include "rendering/ui/editor_section.hpp"

struct GameData;

class GameDataUi
{
public:
    void draw(EditorSection section, GameData &gameData);
};
