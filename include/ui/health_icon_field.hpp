#pragma once

#include <string_view>
#include "ui/inspector_edited.hpp"

struct HealthIconData;

inspector::Edited drawCustomField(std::string_view name, HealthIconData &value);
