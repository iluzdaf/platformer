#pragma once
#include <vector>
#include <unordered_map>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include "game/tile_map/tile.hpp"
class TileMapData;

class TileMap
{
public:
    explicit TileMap(const std::string &jsonFilePath);
    explicit TileMap(const TileMapData &tileMapData);
    void setTileIndex(int x, int y, int tile);
    int getTileIndex(int x, int y) const;
    int getTileIndex(const glm::vec2 &worldPosition) const;
    const Tile &getTile(int tileIndex) const;
    const Tile &getTile(int x, int y) const;
    const Tile &getTile(const glm::vec2 &worldPosition) const;
    int getWidth() const;
    int getHeight() const;
    int getWorldWidth() const;
    int getWorldHeight() const;
    int getTileSize() const;
    void update(float deltaTime);
    glm::vec2 getPlayerStartWorldPosition() const;
    const std::string& getNextLevel() const;

private:
    int width = 0, height = 0, tileSize = 0;
    std::vector<std::vector<int>> tileIndices;
    std::unordered_map<int, Tile> tiles;
    void initByData(const TileMapData &tileMapData);
    glm::ivec2 playerStartPosition = glm::ivec2(0, 0);
    std::string nextLevel = "../assets/tile_maps/level1.json";
};