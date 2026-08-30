#pragma once

#include "physics/aabb.hpp"

struct ActorContactState
{
    bool onGround = false, hitCeiling = false, touchingRightWall = false, touchingLeftWall = false,
         wasOnGround = false, wasHitCeiling = false, wasLastWallLeft = false, ledgeOnLeft = false,
         ledgeOnRight = false, grippableLeftWall = false, grippableRightWall = false;
    AABB collisionAABBX, collisionAABBY;

    bool touchingWall() const
    {
        return touchingLeftWall || touchingRightWall;
    }

    bool grippableWall() const
    {
        return grippableLeftWall || grippableRightWall;
    }
};
