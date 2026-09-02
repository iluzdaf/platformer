#pragma once

#include "animations/frame_animation_data.hpp"

inline constexpr float AnimationPreviewSize = 64.0f;

int previewFrameAt(const FrameAnimationData &animation, double seconds, int whenStill);
