#include <cstddef>
#include <functional>
#include <unordered_map>
#include "ui/fading_aabbs.hpp"
#include "physics/aabb.hpp"

namespace
{
    std::size_t keyOf(const AABB &box)
    {
        glm::ivec2 position = glm::round(box.position * 100.0f);
        glm::ivec2 size = glm::round(box.size * 100.0f);
        std::size_t h1 = std::hash<int>()(position.x) ^ std::hash<int>()(position.y << 1);
        std::size_t h2 = std::hash<int>()(size.x) ^ std::hash<int>()(size.y << 1);
        return h1 ^ (h2 << 1);
    }
}

void FadingAABBs::add(AABB box, ImU32 color, float seconds)
{
    if (box.isEmpty())
    {
        return;
    }

    std::size_t key = keyOf(box);
    auto it = boxes.find(key);
    if (it != boxes.end())
    {
        it->second.secondsLeft = seconds;
    }
    else
    {
        boxes[key] = Fading{box, color, seconds};
    }
}

void FadingAABBs::update(float deltaTime)
{
    for (auto it = boxes.begin(); it != boxes.end();)
    {
        it->second.secondsLeft -= deltaTime;
        if (it->second.secondsLeft <= 0.0f)
            it = boxes.erase(it);
        else
            ++it;
    }
}

const std::unordered_map<std::size_t, FadingAABBs::Fading> &FadingAABBs::all() const
{
    return boxes;
}
