#pragma once

class ImGuiManager;
class Camera2D;
class TileMap;

void drawTileGrid(const ImGuiManager &imGuiManager, const Camera2D &camera, const TileMap &tileMap);

void drawTileInfo(const ImGuiManager &imGuiManager, const Camera2D &camera, const TileMap &tileMap);
