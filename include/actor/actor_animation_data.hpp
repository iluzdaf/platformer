#pragma once

#include <optional>
#include "animations/frame_animation_data.hpp"

struct ActorAnimationData
{
    FrameAnimationData idle;
    std::optional<FrameAnimationData> walk;
    std::optional<FrameAnimationData> dash;
    std::optional<FrameAnimationData> jump;
    std::optional<FrameAnimationData> fall;
    std::optional<FrameAnimationData> wallSlide;
};
