#pragma once

#include <optional>
#include <vector>

class Texture2D;

inline constexpr float TilePickerCellSize = 32.0f;

std::vector<int> tilesToPickFrom(const Texture2D &tileSet, int tileSize);

std::optional<int> drawTilePicker(
    const Texture2D &tileSet,
    int tileSize,
    const std::vector<int> &tileIndices,
    std::optional<int> armed);
