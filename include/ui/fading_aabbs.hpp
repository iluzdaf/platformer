#pragma once

#include <cstddef>
#include <unordered_map>
#include <imgui.h>
#include "physics/aabb.hpp"

class FadingAABBs
{
public:
    struct Fading
    {
        AABB box;
        ImU32 color;
        float secondsLeft;
    };

    void add(AABB box, ImU32 color, float seconds);
    void update(float deltaTime);
    const std::unordered_map<std::size_t, Fading> &all() const;

private:
    std::unordered_map<std::size_t, Fading> boxes;
};
