#pragma once

#include <string>
#include <unordered_map>
#include "game/player/player_data.hpp"
#include "game/npc/npc_data.hpp"
#include "game/tile_map/tile_palette.hpp"
#include "game/debug_data.hpp"
#include "cameras/camera2d_data.hpp"

struct GameData
{
    PlayerData playerData;
    std::unordered_map<std::string, NpcData> npcData;
    TilePalettes tilePalettes;
    Camera2DData cameraData;
    int windowWidth = 800;
    int windowHeight = 600;
    std::string firstLevel = "../assets/levels/level1.json";
    DebugData debugData;
};