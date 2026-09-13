#pragma once

#include <cmath>

// A body at rest does not sit exactly on the line the navigation graph drew.
// Physics leaves the level 6 spider's feet at 161.000015 on a run at 160, so
// any question of the form "am I on this surface" has to allow a settle.
inline constexpr float SettlingTolerance = 4.0f;

inline bool feetSettledOn(float feetY, float surfaceY)
{
    return std::abs(feetY - surfaceY) <= SettlingTolerance;
}
