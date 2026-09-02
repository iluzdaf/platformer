#pragma once

#include <vector>
#include "pickups/pickup.hpp"

struct AABB;

std::vector<Pickup> takeWhatTouches(std::vector<Pickup> &pickups, const AABB &reach);
