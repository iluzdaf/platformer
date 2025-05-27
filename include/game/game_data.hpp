#pragma once
#include "game/player_data.hpp"
#include "game/camera2d_data.hpp"
#include "game/physics_data.hpp"

struct GameData
{
    PlayerData playerData;
    Camera2DData cameraData;
    PhysicsData physicsData;
};