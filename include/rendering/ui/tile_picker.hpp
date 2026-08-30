#pragma once

#include <optional>
#include <vector>

class Texture2D;

std::optional<int> drawTilePicker(
    const Texture2D &tileSet,
    int tileSize,
    const std::vector<int> &tileIndices,
    std::optional<int> selected);
