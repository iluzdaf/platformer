#pragma once

#include <string>
#include <unordered_map>
#include "player/player_data.hpp"
#include "npc/npc_data.hpp"
#include "tile_map/tile_palette.hpp"
#include "cameras/camera2d_data.hpp"

struct GameData
{
    PlayerData playerData;
    std::unordered_map<std::string, NpcData> npcData;
    TilePalettes tilePalettes;
    Camera2DData cameraData;
    int windowWidth = 800;
    int windowHeight = 600;
    bool debug = false;
};

GameData loadGameData();