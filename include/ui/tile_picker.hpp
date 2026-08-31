#pragma once

#include <optional>
#include <vector>

class Texture2D;
class TileMap;

inline constexpr float TilePickerCellSize = 32.0f;

std::vector<int> tilesToPickFrom(const TileMap &tileMap);

std::optional<int> drawTilePicker(
    const Texture2D &tileSet,
    int tileSize,
    const std::vector<int> &tileIndices,
    std::optional<int> armed);
