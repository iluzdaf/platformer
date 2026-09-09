#pragma once
#include <string>
#include <vector>

struct FrameCueData
{
    int frame = 0;
    std::string name;

    bool operator==(const FrameCueData &) const = default;
};

struct FrameAnimationData
{
    std::vector<int> frames;
    float frameDuration = 0.1f;
    std::vector<FrameCueData> cues = {};
    bool loops = true;
};
