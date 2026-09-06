#pragma once

#include <optional>
#include "game/level_resizing.hpp"

inline constexpr float SizeLabelWidth = 96.0f;

std::optional<Resize> drawSizeButtons(int width, int height);
