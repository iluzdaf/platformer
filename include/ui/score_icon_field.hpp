#pragma once

#include <string_view>
#include "ui/inspector_edited.hpp"

struct ScoreIconData;

inspector::Edited drawCustomField(std::string_view name, ScoreIconData &value);
