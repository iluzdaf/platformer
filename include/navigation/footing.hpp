#pragma once

#include <algorithm>
#include <cmath>

inline constexpr float TakeOffReach = 1.5f;

// A climber counts as having reached a node on its wall once its feet are this close.
inline constexpr float ClimbArrivesWithin = 1.0f;

// A body at rest sits on the collider beneath it, give or take float noise. But a run
// crosses steps up to the body's stepHeight, so its feet can be a step from a node of the
// run, or from another body on it, and a climber stops within ClimbArrivesWithin of its
// node. Any question of the form "am I on this surface" allows for both.
inline float settlingTolerance(float stepHeight)
{
    constexpr float FloatNoise = 0.5f;
    return std::max(stepHeight, ClimbArrivesWithin) + FloatNoise;
}

inline bool feetSettledOn(float feetY, float surfaceY, float stepHeight)
{
    return std::abs(feetY - surfaceY) <= settlingTolerance(stepHeight);
}
