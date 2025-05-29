#pragma once
#include "game/player/player_data.hpp"
#include "cameras/camera2d_data.hpp"
#include "physics/physics_data.hpp"

struct GameData
{
    PlayerData playerData;
    Camera2DData cameraData;
    PhysicsData physicsData;
};