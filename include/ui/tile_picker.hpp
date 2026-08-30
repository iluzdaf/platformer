#pragma once

#include <optional>
#include <vector>

class Texture2D;

inline constexpr float TilePickerCellSize = 32.0f;
inline constexpr unsigned int TilePickerArmedColour = 0xFF00FFFF;

std::optional<int> drawTilePicker(
    const Texture2D &tileSet,
    int tileSize,
    const std::vector<int> &tileIndices,
    std::optional<int> selected);
