#include <cstddef>
#include "animations/frame_animation.hpp"
#include "animations/frame_animation_data.hpp"

FrameAnimation::FrameAnimation(const FrameAnimationData &frameAnimationData)
    : frames(frameAnimationData.frames), frameDuration(frameAnimationData.frameDuration)
{
}

void FrameAnimation::update(float deltaTime)
{
    if (frames.empty() || frameDuration <= 0.0f)
        return;

    timer += deltaTime;
    while (timer >= frameDuration)
    {
        timer -= frameDuration;
        currentFrame =
            static_cast<int>((static_cast<std::size_t>(currentFrame) + 1) % frames.size());
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
