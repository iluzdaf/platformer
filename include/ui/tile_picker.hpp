#pragma once

#include <optional>

class Texture2D;
struct SheetData;

inline constexpr float TilePickerCellSize = 32.0f;

bool drawTileCell(const Texture2D &sheet, int cellSize, int tileIndex);

std::optional<int> drawTilePicker(
    const Texture2D &sheet,
    const SheetData &tileSet,
    std::optional<int> armed);
