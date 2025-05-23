#pragma once
#include <vector>

class TileMap
{
public:
    TileMap(int width, int height);
    void setTile(int x, int y, int tile);
    int getTile(int x, int y) const;
    int getWidth() const;
    int getHeight() const;

private:
    int width, height;
    std::vector<std::vector<int>> tiles;
};