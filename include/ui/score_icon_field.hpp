#pragma once

#include <string_view>
#include "ui/inspector_edited.hpp"

struct ScoreIcon;

inspector::Edited drawCustomField(std::string_view name, ScoreIcon &value);
