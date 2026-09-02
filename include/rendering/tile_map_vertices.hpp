#pragma once

#include <vector>

class TileMap;

inline constexpr int VerticesPerTile = 6;
inline constexpr int FloatsPerVertex = 4;

void appendTileMapVertices(
    std::vector<float> &into,
    const TileMap &tileMap,
    int sheetWidth,
    int sheetHeight);
