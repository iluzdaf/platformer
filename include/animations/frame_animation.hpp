#pragma once
#include <vector>
#include "animations/frame_animation_data.hpp"

class FrameAnimation
{
public:
    FrameAnimation() = default;
    FrameAnimation(const FrameAnimationData &frameAnimationData);
    void update(float deltaTime);
    int getCurrentFrame() const;
    void reset();
    FrameAnimationData toFrameAnimationData() const;

private:
    std::vector<int> frames;
    float frameDuration = 0.1f;
    float timer = 0.0f;
    int currentFrame = 0;
};