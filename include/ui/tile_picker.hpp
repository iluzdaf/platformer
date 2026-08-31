#pragma once

#include <optional>
#include <vector>
#include "ui/brush.hpp"

class Texture2D;

inline constexpr float TilePickerCellSize = 32.0f;
inline constexpr unsigned int TilePickerArmedColour = 0xFF00FFFF;

std::optional<Brush> drawTilePicker(
    const Texture2D &tileSet,
    int tileSize,
    const std::vector<Brush> &brushes,
    std::optional<Brush> armed);
