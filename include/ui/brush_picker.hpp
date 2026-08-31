#pragma once

#include <optional>
#include <vector>
#include "ui/brush.hpp"

class Texture2D;

inline constexpr float BrushPickerCellSize = 32.0f;
inline constexpr unsigned int BrushPickerArmedColour = 0xFF00FFFF;

std::optional<Brush> drawBrushPicker(
    const Texture2D &tileSet,
    int tileSize,
    const std::vector<Brush> &brushes,
    std::optional<Brush> armed);
