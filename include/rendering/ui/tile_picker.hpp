#pragma once

#include <vector>

class Texture2D;

int drawTilePicker(
    const Texture2D &tileSet,
    int tileSize,
    const std::vector<int> &tileIndices,
    int selected);
