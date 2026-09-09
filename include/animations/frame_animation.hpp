#pragma once
#include <string>
#include <vector>
#include "animations/frame_animation_data.hpp"

class FrameAnimation
{
public:
    FrameAnimation() = default;
    FrameAnimation(const FrameAnimationData &frameAnimationData);
    void update(float deltaTime);
    int frame() const;
    void reset();
    bool finished() const;
    std::vector<std::string> takeCues();

private:
    std::vector<int> frames;
    float frameDuration = 0.1f;
    std::vector<FrameCueData> cues;
    bool loops = true;
    bool playedOut = false;
    float timer = 0.0f;
    int currentFrame = 0;
    std::vector<int> entered;
};
