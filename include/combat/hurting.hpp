#pragma once

#include "physics/aabb.hpp"

struct Hurting
{
    AABB box;
    int damage = 0;
    float direction = 0.0f;
};
