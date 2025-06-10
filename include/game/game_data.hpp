#pragma once
#include "game/player/player_data.hpp"
#include "game/debug_data.hpp"
#include "cameras/camera2d_data.hpp"
#include "physics/physics_data.hpp"

struct GameData
{
    PlayerData playerData;
    Camera2DData cameraData;
    PhysicsData physicsData;
    int windowWidth = 800;
    int windowHeight = 600;
    std::string firstLevel = "../assets/tile_maps/level1.json";
    DebugData debugData;
};