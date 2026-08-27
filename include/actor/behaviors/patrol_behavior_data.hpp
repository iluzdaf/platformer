#pragma once

struct PatrolBehaviorData
{
    float arrivalThreshold = 2.0f;

    // Whether to leave the platform it starts on. A roamer will use jumps and
    // falls to cross the level, but only to somewhere it can get back from.
    bool roams = false;
};
