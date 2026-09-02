#pragma once

#include <optional>

class Texture2D;
struct TileSet;

inline constexpr float TilePickerCellSize = 32.0f;

bool drawTileCell(const Texture2D &sheet, int cellSize, int tileIndex);

void drawTileImage(const Texture2D &sheet, int cellSize, int tileIndex, float size);

std::optional<int> drawTilePicker(
    const Texture2D &sheet,
    const TileSet &tileSet,
    std::optional<int> armed);
