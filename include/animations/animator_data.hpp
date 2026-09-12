#pragma once

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include "animations/animation_ladder_data.hpp"
#include "animations/frame_animation_data.hpp"

struct AnimatorData
{
    std::map<std::string, FrameAnimationData> clips;
    AnimationLadderData ladder;
    std::string startClip;
};

inline const FrameAnimationData *clipNamed(const AnimatorData &animations, std::string_view name)
{
    auto found = animations.clips.find(std::string(name));
    return found == animations.clips.end() ? nullptr : &found->second;
}

std::optional<std::string> whyNotAnAnimator(const AnimatorData &data);