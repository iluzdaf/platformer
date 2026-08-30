#include <cstddef>
#include <unordered_map>
#include "rendering/ui/fading_aabbs.hpp"
#include "physics/aabb.hpp"

void FadingAABBs::add(AABB box, ImU32 color, float seconds)
{
    if (box.isEmpty())
    {
        return;
    }

    std::size_t hash = box.hash();
    auto it = boxes.find(hash);
    if (it != boxes.end())
    {
        it->second.secondsLeft = seconds;
    }
    else
    {
        boxes[hash] = Fading{box, color, seconds};
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
