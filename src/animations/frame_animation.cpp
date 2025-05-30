#include <cassert>
#include "animations/frame_animation.hpp"
#include "animations/frame_animation_data.hpp"

FrameAnimation::FrameAnimation(const FrameAnimationData &frameAnimationData)
    : frames(std::move(frameAnimationData.frames)),
      frameDuration(frameAnimationData.frameDuration)
{
    assert(frameDuration > 0);
}

void FrameAnimation::update(float deltaTime)
{
    if (frames.empty())
        return;

    timer += deltaTime;
    while (timer >= frameDuration)
    {
        timer -= frameDuration;
        currentFrame = (currentFrame + 1) % frames.size();
    }
}

int FrameAnimation::getCurrentFrame() const
{
    return frames.empty() ? 0 : frames[currentFrame];
}

void FrameAnimation::reset()
{
    currentFrame = 0;
    timer = 0.0f;
}