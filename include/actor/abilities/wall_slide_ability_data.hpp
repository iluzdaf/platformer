#pragma once

struct WallSlideAbilityData
{
    float slideSpeed = 50;

    bool operator==(const WallSlideAbilityData &) const = default;
};