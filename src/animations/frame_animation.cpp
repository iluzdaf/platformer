#include <cstddef>
#include <string>
#include <vector>
#include "animations/frame_animation.hpp"
#include "animations/frame_animation_data.hpp"

FrameAnimation::FrameAnimation(const FrameAnimationData &frameAnimationData)
    : frames(frameAnimationData.frames), frameDuration(frameAnimationData.frameDuration),
      cues(frameAnimationData.cues), loops(frameAnimationData.loops)
{
    reset();
}

void FrameAnimation::update(float deltaTime)
{
    if (frames.empty() || frameDuration <= 0.0f)
        return;

    timer += deltaTime;
    while (timer >= frameDuration)
    {
        timer -= frameDuration;
        bool onTheLast = static_cast<std::size_t>(currentFrame) + 1 == frames.size();
        if (onTheLast && !loops)
        {
            playedOut = true;
            timer = 0.0f;
            return;
        }

        currentFrame =
            static_cast<int>((static_cast<std::size_t>(currentFrame) + 1) % frames.size());
        entered.push_back(currentFrame);
    }
}

bool FrameAnimation::finished() const
{
    return playedOut;
}

int FrameAnimation::frame() const
{
    return frames.empty() ? 0 : frames[currentFrame];
}

void FrameAnimation::reset()
{
    currentFrame = 0;
    timer = 0.0f;
    playedOut = false;
    entered.clear();
    if (!frames.empty())
        entered.push_back(0);
}

std::vector<std::string> FrameAnimation::takeCues()
{
    std::vector<std::string> said;
    for (int position : entered)
        for (const FrameCueData &cue : cues)
            if (cue.frame == position)
                said.push_back(cue.name);

    entered.clear();
    return said;
}
