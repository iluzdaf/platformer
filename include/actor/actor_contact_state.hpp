#pragma once

#include "physics/aabb.hpp"

struct ActorContactState
{
    bool onGround = false, hitCeiling = false, touchingRightWall = false, touchingLeftWall = false,
         wasOnGround = false, wasHitCeiling = false, wasLastWallLeft = false, ledgeOnLeft = false,
         ledgeOnRight = false;
    AABB collisionAABBX, collisionAABBY;
};
