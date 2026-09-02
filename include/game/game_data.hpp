#pragma once

#include <string>
#include <map>
#include "player/player_data.hpp"
#include "npc/npc_data.hpp"
#include "pickups/pickup_data.hpp"
#include "tile_map/tile_palette.hpp"
#include "cameras/camera2d_data.hpp"

struct GameSettingsData
{
    int windowWidth = 800;
    int windowHeight = 600;
    bool debug = false;
};

struct GameData
{
    GameSettingsData settings;
    Camera2DData cameraData;
    PlayerData playerData;
    std::map<std::string, NpcData> npcData;
    std::map<std::string, PickupData> pickupData;
    TilePalettes tilePalettes;
};

GameData loadGameData();

void saveGameSettings(const GameSettingsData &settings);
void saveCameraData(const Camera2DData &cameraData);
void savePlayerData(const PlayerData &playerData);
void saveNpcData(const std::map<std::string, NpcData> &npcData);
void savePickupData(const std::map<std::string, PickupData> &pickupData);
void saveTilePalettes(const TilePalettes &tilePalettes);