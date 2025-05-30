#pragma once
#include <vector>
class FrameAnimationData;

class FrameAnimation
{
public:
    FrameAnimation() = default;
    FrameAnimation(const FrameAnimationData &frameAnimationData);
    void update(float deltaTime);
    int getCurrentFrame() const;
    void reset();

private:
    std::vector<int> frames;
    float frameDuration = 0.1f;
    float timer = 0.0f;
    int currentFrame = 0;
};