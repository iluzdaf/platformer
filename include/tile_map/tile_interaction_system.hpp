#pragma once
class Player;
class TileMap;

class TileInteractionSystem
{
public:
    void fixedUpdate(Player &player, TileMap &tileMap);
};