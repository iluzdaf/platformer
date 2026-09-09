#pragma once

#include <map>
#include <string>
#include <string_view>
#include "animations/animator_data.hpp"
#include "animations/frame_animation_data.hpp"

inline constexpr std::string_view IdleClip = "idle";
inline constexpr std::string_view AttackClip = "attack";

struct ActorAnimationData
{
    std::map<std::string, FrameAnimationData> clips;
    AnimatorData ladder;
};

inline const FrameAnimationData *clipNamed(
    const ActorAnimationData &animations,
    std::string_view name)
{
    auto found = animations.clips.find(std::string(name));
    return found == animations.clips.end() ? nullptr : &found->second;
}
