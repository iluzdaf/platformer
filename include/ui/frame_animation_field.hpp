#pragma once

#include "animations/frame_animation_data.hpp"

inline constexpr float AnimationPreviewSize = 64.0f;

inline constexpr float PickerWidth = 240.0f;
inline constexpr float PickerHeight = 180.0f;

int previewFrameAt(const FrameAnimationData &animation, double seconds, int whenStill);
