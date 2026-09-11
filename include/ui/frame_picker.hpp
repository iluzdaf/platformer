#pragma once

#include <string_view>
#include "ui/inspector_edited.hpp"

struct SheetInScope;

inline constexpr float PickerWidth = 240.0f;
inline constexpr float PickerHeight = 180.0f;

inspector::Edited drawFramePicked(const SheetInScope &offering, int &frame);

inspector::Edited drawFrameField(std::string_view name, int &frame);
