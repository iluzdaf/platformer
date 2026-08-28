#pragma once

#include "physics/aabb.hpp"

struct ActorContactState
{
    bool onGround = false, hitCeiling = false, touchingRightWall = false, touchingLeftWall = false,
         wasOnGround = false, wasHitCeiling = false, wasLastWallLeft = false;
    AABB collisionAABBX, collisionAABBY;
};
