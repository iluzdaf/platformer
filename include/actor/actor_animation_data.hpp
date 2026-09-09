#pragma once

#include <optional>
#include "animations/animator_data.hpp"
#include "animations/frame_animation_data.hpp"

struct ActorAnimationData
{
    FrameAnimationData idle;
    std::optional<FrameAnimationData> walk;
    std::optional<FrameAnimationData> dash;
    std::optional<FrameAnimationData> jump;
    std::optional<FrameAnimationData> fall;
    std::optional<FrameAnimationData> wallSlide;
    std::optional<FrameAnimationData> climb;
    std::optional<FrameAnimationData> attack;
    std::optional<FrameAnimationData> knockback;
    std::optional<FrameAnimationData> dead;
    std::optional<AnimatorData> ladder;
};
