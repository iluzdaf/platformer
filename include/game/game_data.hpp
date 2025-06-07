#pragma once
#include "game/player/player_data.hpp"
#include "cameras/camera2d_data.hpp"
#include "physics/physics_data.hpp"
#include "rendering/debug_renderer_data.hpp"

struct GameData
{
    PlayerData playerData;
    Camera2DData cameraData;
    PhysicsData physicsData;
    int screenWidth = 800;
    int screenHeight = 600;
    std::string firstLevel = "../assets/tile_maps/level1.json";
    DebugRendererData debugRendererData;
};